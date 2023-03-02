# UPDATE

<!-- example update -->

UPDATE changes [row-wise](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) attribute values of existing documents in a specified table with new values. Note that you can't update contents of a fulltext field or a columnar attribute. If there's such a need, use [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md).

Attribute updates are supported for RT, PQ and plain tables. All attribute types can be updated as long as they are stored in the [traditional row-wise storage](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages).


**Note that document id cannot be updated.**

**Note that when you update an attribute, its [secondary index](../../Server_settings/Searchd.md#secondary_indexes) gets disabled, so consider [replacing](../../Data_creation_and_modification/Updating_documents/REPLACE.md) the document instead.**

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
UPDATE products SET enabled=0 WHERE id=10;
```

<!-- response -->

```sql
Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update

{
  "index":"products",
  "id":10,
  "doc":
  {
    "enabled":0
  }
}
```

<!-- response JSON -->
```JSON
{
  "_index":"products",
  "updated":1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->updateDocument([
    'enabled'=>0
],10);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [_id] => 10
    [result] => updated
)
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.update({"index" : "products", "id" : 1, "doc" : {"price":10}})
```

<!-- response Python -->
```python
{'id': 1, 'index': 'products', 'result': 'updated', 'updated': None}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"index" : "products", "id" : 1, "doc" : {"price":10}});
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":1,"result":"updated"}
```
<!-- intro -->
##### java:

<!-- request Java -->
``` java
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("price",10);
}};
updateRequest.index("products").id(1L).setDoc(doc);
indexApi.update(updateRequest);
```

<!-- response Java -->
```java
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- end -->

<!-- example update multiple attributes -->

Multiple attributes can be updated in a single statement.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
UPDATE products
SET price=100000000000,
    coeff=3465.23,
    tags1=(3,6,4),
    tags2=()
WHERE MATCH('phone') AND enabled=1;
```

<!-- response -->

```sql
Query OK, 148 rows affected (0.0 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update
{
  "index":"products",
  "doc":
  {
    "price":100000000000,
    "coeff":3465.23,
    "tags1":[3,6,4],
    "tags2":[]
  },
  "query":
  {
	"match": { "*": "phone" },
	"equals": { "enabled": 1 }
  }
}
```

<!-- response JSON -->
```JSON
{
  "_index":"products",
  "updated":148
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$query= new BoolQuery();
$query->must(new Match('phone','*'));
$query->must(new Equals('enabled',1));
$index->updateDocuments([
    'price' => 100000000000,
    'coeff' => 3465.23,
    'tags1' => [3,6,4],
    'tags2' => []
    ],
    $query
);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [updated] => 148
)
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.update({"index" : "products", "id" : 1, "doc" : {
    "price": 100000000000,
    "coeff": 3465.23,
    "tags1": [3,6,4],
    "tags2": []}})
```

<!-- response Python -->
```python
{'id': 1, 'index': 'products', 'result': 'updated', 'updated': None}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"index" : "products", "id" : 1, "doc" : {
    "price": 100000000000,
    "coeff": 3465.23,
    "tags1": [3,6,4],
    "tags2": []}});
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":1,"result":"updated"}
```
<!-- intro -->
##### java:

<!-- request Java -->
``` java
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("price",10);
    put("coeff",3465.23);
    put("tags1",new int[]{3,6,4});
    put("tags2",new int[]{});
}};
updateRequest.index("products").id(1L).setDoc(doc);
indexApi.update(updateRequest);
```

<!-- response Java -->
```java
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- end -->

When assigning out-of-range values to 32-bit attributes, they will be trimmed to their lower 32 bits without a prompt. For example, if you try to update the 32-bit unsigned int with a value of 4294967297, the value of 1 will actually be stored, because the lower 32 bits of 4294967297 (0x100000001 in hex) amount to 1 (0x00000001 in hex).

<!-- example partial JSON update -->

`UPDATE` can be used to perform partial JSON updates on numeric data types or arrays of numeric data types. Just make sure you don't update an integer value with a float value as it will be rounded off.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
insert into products (id, title, meta) values (1,'title','{"tags":[1,2,3]}');

update products set meta.tags[0]=100 where id=1;
```

<!-- response -->

```sql
Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /insert
{
	"index":"products",
	"id":100,
	"doc":
	{
		"title":"title",
		"meta": {
            "tags":[1,2,3]
        }
	}
}

POST /update
{
	"index":"products",
	"id":100,
	"doc":
	{
		"meta.tags[0]":100
	}
}
```

<!-- response JSON -->
```JSON
{
   "_index":"products",
   "_id":100,
   "created":true,
   "result":"created",
   "status":201
}

{
  "_index":"products",
  "updated":1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->insertDocument([
    'title' => 'title',
    'meta' => ['tags' => [1,2,3]]
],1);
$index->updateDocument([
    'meta.tags[0]' => 100
],1);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [_id] => 1
    [created] => true
    [result] => created
)

Array(
    [_index] => products
    [updated] => 1
)
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.update({"index" : "products", "id" : 1, "doc" : {
    "meta.tags[0]": 100}})
```

<!-- response Python -->
```python
{'id': 1, 'index': 'products', 'result': 'updated', 'updated': None}
```

<!-- intro -->
##### javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"index" : "products", "id" : 1, "doc" : {
   "meta.tags[0]": 100}});
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":1,"result":"updated"}
```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("meta.tags[0]",100);
}};
updateRequest.index("products").id(1L).setDoc(doc);
indexApi.update(updateRequest);
```

<!-- response Java -->
```java
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- end -->

<!-- example full JSON update -->

Updating other data types or changing property type in a JSON attribute requires a full JSON update.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
insert into products values (1,'title','{"tags":[1,2,3]}');

update products set data='{"tags":["one","two","three"]}' where id=1;
```

<!-- response -->

```sql
Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /insert
{
	"index":"products",
	"id":1,
	"doc":
	{
		"title":"title",
		"data":"{\"tags\":[1,2,3]}"
	}
}

POST /update
{
	"index":"products",
	"id":1,
	"doc":
	{
		"data":"{\"tags\":[\"one\",\"two\",\"three\"]}"
	}
}
```

<!-- response JSON -->
```JSON
{
  "_index":"products",
  "updated":1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->insertDocument([
    'title'=> 'title',
    'data' => [
         'tags' => [1,2,3]
    ]
],1);

$index->updateDocument([
    'data' => [
            'one', 'two', 'three'
    ]
],1);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [_id] => 1
    [created] => true
    [result] => created
)

Array(
    [_index] => products
    [updated] => 1
)
```
<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi.insert({"index" : "products", "id" : 100, "doc" : {"title" : "title", "meta" : {"tags":[1,2,3]}}})
indexApi.update({"index" : "products", "id" : 100, "doc" : {"meta" : {"tags":['one','two','three']}}})
```

<!-- response Python -->
```python

{'created': True,
 'found': None,
 'id': 100,
 'index': 'products',
 'result': 'created'}
{'id': 100, 'index': 'products', 'result': 'updated', 'updated': None}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.insert({"index" : "products", "id" : 100, "doc" : {"title" : "title", "meta" : {"tags":[1,2,3]}}});
res = await indexApi.update({"index" : "products", "id" : 100, "doc" : {"meta" : {"tags":['one','two','three']}}});
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":100,"created":true,"result":"created"}
{"_index":"products","_id":100,"result":"updated"}

```


<!-- intro -->
##### java:

<!-- request Java -->
``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
doc = new HashMap<String,Object>(){{
    put("title","title");
    put("meta",
        new HashMap<String,Object>(){{
            put("tags",new int[]{1,2,3});
        }});

}};
newdoc.index("products").id(100L).setDoc(doc);        
indexApi.insert(newdoc);

updatedoc = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("meta",
        new HashMap<String,Object>(){{
            put("tags",new String[]{"one","two","three"});
        }});
}};
updatedoc.index("products").id(100L).setDoc(doc);
indexApi.update(updatedoc);

```

<!-- response Java -->
```java
class SuccessResponse {
    index: products
    id: 100
    created: true
    result: created
    found: null
}

class UpdateResponse {
    index: products
    updated: null
    id: 100
    result: updated
}
```

<!-- end -->

<!-- example cluster update -->

When using replication, the table name should be prepended with `cluster_name:` (in SQL) so that updates will be propagated to all nodes in the cluster. For queries via HTTP you should set a `cluster` property. See [setting up replication](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) for more info.

```json
{
  "cluster":"nodes4",
  "index":"test",
  "id":1,
  "doc":
  {
    "gid" : 100,
    "price" : 1000
  }
}
```

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
update weekly:posts set enabled=0 where id=1;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update
{
	"cluster":"weekly",
	"index":"products",
	"id":1,
	"doc":
	{
		"enabled":0
	}
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->setCluster('weekly');
$index->updateDocument(['enabled'=>0],1);
```
<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi.update({"cluster":"weekly", "index" : "products", "id" : 1, "doc" : {"enabled" : 0}})

```
<!-- intro -->
##### javascript:

<!-- request javascript -->
``` javascript
res = wait indexApi.update({"cluster":"weekly", "index" : "products", "id" : 1, "doc" : {"enabled" : 0}});

```
<!-- intro -->
##### java:

<!-- request Java -->
``` java
updatedoc = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("enabled",0);
}};
updatedoc.index("products").cluster("weekly").id(1L).setDoc(doc);
indexApi.update(updatedoc);

```

<!-- response Java -->
```java
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- end -->


## Updates via SQL

Here is the syntax for the SQL `UPDATE` statement:

```sql
UPDATE table SET col1 = newval1 [, ...] WHERE where_condition [OPTION opt_name = opt_value [, ...]] [FORCE|IGNORE INDEX(id)]
```


`where_condition` has the same syntax as in the [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) statement.

<!-- example MVA empty update -->

Multi-value attribute value sets must be specified as comma-separated lists in parentheses. To remove all values from a multi-value attribute, just assign `()` to it.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
UPDATE products SET tags1=(3,6,4) WHERE id=1;

UPDATE products SET tags1=() WHERE id=1;
```

<!-- response SQL -->

```sql
Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update

{
	"index":"products",
	"_id":1,
	"doc":
	{
		"tags1": []
	}
}
```

<!-- response JSON -->
```JSON
{
  "_index":"products",
  "updated":1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->updateDocument(['tags1'=>[]],1);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [updated] => 1
)
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python

indexApi.update({"index" : "products", "id" : 1, "doc" : {"tags1": []}})
```

<!-- response Python -->
```python
{'id': 1, 'index': 'products', 'result': 'updated', 'updated': None}
```

<!-- intro -->
##### javascript:

<!-- request javascript -->
``` javascript

indexApi.update({"index" : "products", "id" : 1, "doc" : {"tags1": []}})
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":1,"result":"updated"}
```
<!-- intro -->
##### java:

<!-- request Java -->
``` java
updatedoc = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("tags1",new int[]{});
}};
updatedoc.index("products").id(1L).setDoc(doc);
indexApi.update(updatedoc);

```

<!-- response Java -->
```java
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```
<!-- end -->


`OPTION` clause is a Manticore-specific extension that lets you control a number of per-update options. The syntax is:

```sql
OPTION <optionname>=<value> [ , ... ]
```

The options are the same as for [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) statement. Specifically for `UPDATE` statement you can use these options:

*   'ignore_nonexistent_columns' - If set to **1** points that the update will silently ignore any warnings about trying to update a column which is not exists in current table schema. Default value is  **0**.
*   'strict' - this option is used in partial JSON attribute updates. By default (strict=1), `UPDATE` will end in an error if the `UPDATE` query tries to perform an update on non-numeric properties. With strict=0 if multiple properties are updated and some are not allowed, the `UPDATE` will not end in error and will perform the changes only on allowed properties (with the rest being ignored). If none of the `SET` changes of the `UPDATE` are not permitted, the command will end in an error even with strict=0.

### Query optimizer hints

In rare cases, Manticore's built-in query analyzer may be incorrect in understanding a query and determining whether a table by ID should be used. This can result in poor performance for queries like `UPDATE ... WHERE id = 123`.
For information on how to force the optimizer to use a docid index, see [Query optimizer hints](../../Searching/Options.md#Query optimizer hints).

## Updates via HTTP JSON

Updates using HTTP JSON protocol are performed via the `/update` endpoint. The syntax is similar to the [/insert endpoint](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md), but this time the `doc` property is mandatory.

The server will respond with a JSON object stating if the operation was successful or not.

<!-- example JSON update -->

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update
{
  "index":"test",
  "id":1,
  "doc":
   {
     "gid" : 100,
     "price" : 1000
    }
}
```

<!-- response JSON -->
``` JSON
{
  "_index": "test",
  "_id": 1,
  "result": "updated"
}
```

<!-- end -->

<!-- example JSON Example_2 -->

The id of the document that needs to be updated can be set directly using the `id` property (as in the example above) or you can do an update by query and apply the update to all the documents that match the query:

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /update

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
```

<!-- response JSON -->

```json
{
  "_index":"products",
  "updated":1
}
```

<!-- end -->

The query syntax is the same as in the [/search endpoint](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON). Note that you can't specify `id` and `query` at the same time.

## Flushing attributes

```sql
FLUSH ATTRIBUTES
```

Flushes all in-memory attribute updates in all the active disk tables to disk. Returns a tag that identifies the result on-disk state (basically, a number of actual disk attribute saves performed since the server startup).

```sql
mysql> UPDATE testindex SET channel_id=1107025 WHERE id=1;
Query OK, 1 row affected (0.04 sec)

mysql> FLUSH ATTRIBUTES;
+------+
| tag  |
+------+
|    1 |
+------+
1 row in set (0.19 sec)
```
See also [attr_flush_period](../../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) setting.


## Bulk updates

<!-- example bulk update -->

Several update operations can be performed in a single call using the `/bulk` endpoint. This endpoint only works with data that has `Content-Type` set to `application/x-ndjson`. The data itself should be formatted as a newline-delimited json (NDJSON). Basically it means that each line should contain exactly one json statement and end with a newline `\n` and maybe a `\r`.

<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "update" : { "index" : "products", "id" : 1, "doc": { "price" : 10 } } }
{ "update" : { "index" : "products", "id" : 2, "doc": { "price" : 20 } } }
```

<!-- response JSON -->

```json
{
   "items":
   [
      {
         "update":
         {
            "_index":"products",
            "_id":1,
            "result":"updated"
         }
      },
      {
         "update":
         {
            "_index":"products",
            "_id":2,
            "result":"updated"
         }
      }
   ],
   "errors":false
}
```

<!-- end -->

`/bulk` endpoint supports inserts, replaces and deletes. Each statement starts with an action type (in this case, `update`). Here's a list of the supported actions:

* `insert`: Inserts a document. The syntax is the same as in the [/insert endpoint](../../Quick_start_guide.md#Add-documents).
* `create`: a synonym for `insert`
* `replace`: Replaces a document. The syntax is the same as in the [/replace](../../Data_creation_and_modification/Updating_documents/REPLACE.md).
* `index`: a synonym for `replace`
* `update`: Updates a document. The syntax is the same as in the [/update](../../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON).
* `delete`: Deletes a document. The syntax is the same as in the [/delete endpoint](../../Data_creation_and_modification/Deleting_documents.md).

Updates by query and deletes by query are also supported.

<!-- example bulk by query -->

<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "update" : { "index" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } }
{ "update" : { "index" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } }
```

<!-- response JSON -->

```json
{
  "items":
  [
    {
      "update":
      {
        "_index":"products",
        "updated":0
      }
    },
    {
      "update":
      {
        "_index":"products",
        "updated":3
      }
    }
  ],
  "errors":false
}
```
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php

$client->bulk([
    ['update'=>[
            'index' => 'products',
             'doc' => [
                'coeff' => 100
            ],
            'query' => [
                'range' => ['price'=>['gte'=>1000]]
            ]   
        ]
    ],
    ['update'=>[
            'index' => 'products',
             'doc' => [
                'coeff' => 0
            ],
            'query' => [
                'range' => ['price'=>['lt'=>1000]]
            ]   
        ]
    ]
]);
```

<!-- response PHP -->

```php
Array(
    [items] => Array (
        Array(
            [update] => Array(
                [_index] => products
                [updated] => 0
            )
        )   
        Array(
             [update] => Array(
                 [_index] => products
                 [updated] => 3
             )
        )    
)

```


<!-- intro -->
##### Python:

<!-- request Python -->
``` python
docs = [ \
            { "update" : { "index" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } }, \
            { "update" : { "index" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } } ]
indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{'error': None,
 'items': [{u'update': {u'_index': u'products', u'updated': 1}},
           {u'update': {u'_index': u'products', u'updated': 3}}]}

```
<!-- intro -->
##### javascript:

<!-- request javascript -->
``` javascript
docs = [
            { "update" : { "index" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } },
            { "update" : { "index" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } } ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

<!-- response javascript -->
```javascript
{"items":[{"update":{"_index":"products","updated":1}},{"update":{"_index":"products","updated":5}}],"errors":false}

```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
String   body = "{ \"update\" : { \"index\" : \"products\", \"doc\": { \"coeff\" : 1000 }, \"query\": { \"range\": { \"price\": { \"gte\": 1000 } } } }} "+"\n"+
    "{ \"update\" : { \"index\" : \"products\", \"doc\": { \"coeff\" : 0 }, \"query\": { \"range\": { \"price\": { \"lt\": 1000 } } } } }"+"\n";         
indexApi.bulk(body);
```

<!-- response Java -->
```java
class BulkResponse {
    items: [{replace={_index=products, _id=1, created=false, result=updated, status=200}}, {replace={_index=products, _id=2, created=false, result=updated, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- end -->



Note that the bulk operation stops at the first query that results in an error.

## Settings related with updates

#### attr_update_reserve

```ini
attr_update_reserve=size
```

<!-- example attr_update_reserve -->
`attr_update_reserve` is a per-table setting which sets the space to be reserved for blob attribute updates. Optional, default value is 128k.

When blob attributes (MVAs, strings, JSON), are updated, their length may change. If the updated string (or MVA, or JSON) is shorter than the old one, it overwrites the old one in the `.spb` file. But if the updated string is longer, updates are written to the end of the `.spb` file. This file is memory mapped, that's why resizing it may be a rather slow process, depending on the OS implementation of memory mapped files.

To avoid frequent resizes, you can specify the extra space to be reserved at the end of the `.spb` file by using this option.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
create table products(title text, price float) attr_update_reserve = '1M'
```

<!-- request JSON -->

```JSON
POST /cli -d "
create table products(title text, price float) attr_update_reserve = '1M'"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'settings' => [
            'attr_update_reserve' => '1M'
        ],
        'columns' => [
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ]
    ],
    'index' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('create table products(title text, price float) attr_update_reserve = \'1M\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('create table products(title text, price float) attr_update_reserve = \'1M\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("create table products(title text, price float) attr_update_reserve = '1M'");
```
<!-- request CONFIG -->

```ini
table products {
  attr_update_reserve = 1M
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->


#### attr_flush_period

```
attr_flush_period = 900 # persist updates to disk every 15 minutes
```

When updating attributes the changes are first written to in-memory copy of attributes. This setting allows to set the interval between flushing the updates to disk. It defaults to 0, which disables the periodic flushing, but flushing will still occur at normal shut-down.
