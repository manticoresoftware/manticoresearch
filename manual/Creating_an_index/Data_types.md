# Data types

## Full-text fields and attributes

Manticore's data types can be split into full-text fields and attributes.

Full-text fields are indexed and can be searched for keywords. They cannot be used in filtering, sorting or grouping. However, original document content can be retrieved and used in result set highlighting.

Full-text fields are represented by the `Text` data type. All the other data types are attributes.

Attributes are additional values associated with each document that can be used to perform additional filtering and sorting during search.

It is often desired to additionally process full-text search results based not only on matching document ID and its rank, but on a number of other per-document values as well. For instance, one might need to sort news search results by date and then relevance, or search through products within specified price range, or limit blog search to posts made by selected users, or group results by month. To do that efficiently, Manticore allows to attach a number of additional attributes to each document. It's then possible to use stored values to filter, sort, or group full-text matches.

Attributes, unlike the fields, are not full-text indexed. They are stored in the index, but it is not possible to search them as full-text.

<!-- example attributes or fields -->

A good example for attributes would be a forum posts index. Assume that only title and content fields need to be full-text searchable - but that sometimes it is also required to limit search to a certain author or a sub-forum (ie. search only those rows that have some specific values of author_id or forum_id); or to sort matches by post_date column; or to group matching posts by month of the post_date and calculate per-group match counts.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table forum(title text, content text, author_id int, forum_id int, post_date timestamp);
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table forum(title text, content text, author_id int, forum_id int, post_date timestamp)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('forum');
$index->create([
    'title'=>['type'=>'text'],
	'content'=>['type'=>'text'],
	'author_id'=>['type'=>'int'],
	'forum_id'=>['type'=>'int'],
	'post_date'=>['type'=>'timestamp']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```php
utilsApi.sql('mode=raw&query=create table forum(title text, content text, author_id int, forum_id int, post_date timestamp)')
```

<!-- intro -->
##### config:

<!-- request config -->

```config
index forum
{
	type = rt
	path = forum

	# when configuring fields via config, they are indexed (and not stored) by default
	rt_field = title
	rt_field = content

	# this option needs to be specified for the field to be stored
	stored_fields = title, content

	rt_attr_uint = author_id
	rt_attr_uint = forum_id
	rt_attr_timestamp = post_date
}
```

<!-- end -->

<!-- example filtered query -->

This example shows running a full-text query filtered by `author_id`, `forum_id` and sorted by `post_date`.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from forum where author_id=123 and forum_id in (1,3,7) order by post_date desc
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /search
{
  "index": "forum",
  "query":
  {
    "match_all": {},
    "bool":
    {
      "must":
      [
        { "equals": { "author_id": 123 } },
		{ "in": { "forum_id": [1,3,7] } }
      ]
    }
  },
  "sort": [ { "post_date": "desc" } ]
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->search([
        'index' => 'forum',
        'query' => 
        [
            'match_all' => [],
            'bool' => [
                'must' => [
                    'equals' => ['author_id' => 123],
                    'in' => [
                        'forum_id' => [
                            1,3,7
                        ]
                    ]
                ]
            ]
        ],
        'sort' => [
        ['post_date' => 'desc']
    ]
]);
```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"index":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]})
```

<!-- end -->

Below is the list of data types supported by Manticore Search:

## Document ID

The identifier of a document in the index. Document IDs must be **unique signed positive non-zero 64-bit integers**. Note that no negative or zero values are allowed. Document IDs are implicit and have no declaration. Update operation is forbidden on document ids.

## Text

<!-- example working with text -->

Text data type forms the full-text part of the index. Text fields are indexed and can be searched for keywords.

Text is passed through an analyzer pipeline that converts the text to words, applies morphology transformations etc. Eventually a full-text index (a special data structure that enables quick searches for a keyword) gets built from that text.

Full-text fields can only be used in `MATCH()` clause and cannot be used for sorting or aggregation. Words are stored in an inverted index along with references to the fields they belong and positions in the field. This allows to search a word inside each field and to use advanced operators like proximity. By default the original text of the fields is both indexed and stored in document storage. It means that the original text can be returned with the query results and it can be used in [search result highlighting](Searching/Highlighting.md).

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text);
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products(title text)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text)')
```

<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	# when configuring fields via config, they are indexed (and not stored) by default
	rt_field = title

	# this option needs to be specified for the field to be stored
	stored_fields = title
}
```

<!-- end -->

<!-- example working with indexed only  -->

This behavior can overridden by explicitly specifying that the text is only indexed.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text indexed);
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products(title text indexed)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text','options'=>['indexed']]
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text indexed)')
```
<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	# when configuring fields via config, they are indexed (and not stored) by default
	rt_field = title
}
```

<!-- end -->

<!-- example for field naming  -->

Fields are named, and you can limit your searches to a single field (e.g. search through "title" only) or a subset of fields (eg. to "title" and "abstract" only). Manticore index format generally supports up to 256 fields.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products where match('@title first');
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /search
{
	"index": "products",
	"query":
	{
		"match": { "title": "first" }
	}
}
```



<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('@title')->get();

```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"index":"products","query":{"match":{"title":"first"}}})
```


<!-- end -->

## String

<!-- example for string attributes  -->

Unlike full-text fields, string attributes are stored as they are received and cannot be used in full-text searches. Instead they are returned in results, they can be used in `WHERE` clause for comparison filtering or `REGEX` and they can be used for sorting and aggregation. In general it's not recommended to store large texts in string attributes, but use string attributes for metadata like names, titles, tags, keys.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text, keys string);
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products(title text, keys string)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'keys'=>['type'=>'string']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text, keys string)')
```
<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_string = keys
}
```

<!-- end -->

<details>
<summary>More</summary>

<!-- example string field -->

You can create a fulltext field that is also stored as a string attribute. This approach creates a fulltext field and a string attribute that have the same name. Note that you can't add a `stored` property to store the data as a a string attribute and in the document storage at the same time.

<!-- intro -->
##### SQL:
<!-- request SQL -->
`string attribute indexed` means that we're working with a string data type that is stored as an attribute and indexed as a full-text field.

```sql
create table products ( title string attribute indexed );
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products ( title string attribute indexed )"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'string','options'=>['indexed','attribute']]
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products ( title string attribute indexed )')
```

<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	rt_field = title
	rt_attr_string = title
}
```

<!-- end -->

</details>

## Integer

<!-- example for integers  -->

Integer type allows storing 32 bit **unsigned** integer values.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text, price int);
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products(title text, price int)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'price'=>['type'=>'int']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text, price int)')
```

<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_uint = type
}
```

<!-- end -->

<!-- example for bit integers  -->

Integers can be stored in shorter sizes than 32 bit by specifying a bit count. For example if we want to store a numeric value which we know is not going to be bigger than 8, the type can be defined as `bit(3)`. Bitcount integers perform slower than the full size ones, but they require less RAM. They are saved in 32-bit chunks, so in order to save space they should be grouped at the end of attributes definitions (otherwise a bitcount integer between 2 full-size integers will occupy 32 bits as well).

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text, flags bit(3), tags bit(2) );
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products(title text, flags bit(3), tags bit(2))"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'flags'=>['type'=>'bit(3)'],
	'tags'=>['type'=>'bit(2)']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text, flags bit(3), tags bit(2) ')
```

<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_uint = flags:3
	rt_attr_uint = tags:2
}
```

<!-- end -->

## Big Integer

<!-- example for bigints  -->

Big integers are 64-bit wide **signed** integers.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text, price bigint );
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products(title text, price bigint)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'price'=>['type'=>'bigint']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text, price bigint )')
```

<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_bigint = type
}
```

<!-- end -->

## Timestamps

<!-- example for timestamps  -->

Timestamp type represents unix timestamps which is stored as a 32-bit integer. The difference is that [time and date](Functions/Date_and_time_functions.md) functions are available for the timestamp type.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text, date timestamp);
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products(title text, date timestamp)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'date'=>['type'=>'timestamp']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text, date timestamp)')
```


<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_timestamp = date
}
```

<!-- end -->

## Float

<!-- example for float -->
Real numbers are stored as 32-bit IEEE 754 single precision floats.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text, coeff float);
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products(title text, coeff float)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'coeff'=>['type'=>'float']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text, coeff float)')
```


<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_float = coeff
}
```

<!-- end -->

<!-- example for eps comparison -->

Unlike integer types, equal comparison of floats is forbidden due to rounding errors. A near equal can be used instead, by checking the absolute error margin.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select abs(a-b)<=0.00001 from products
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->
```http
POST /search
{
  "index": "products",
  "query": { "match_all": {} } },
  "expressions": { "eps": "abs(a-b)" }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->expression('eps','abs(a-b)')->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"index":"products","query":{"match_all":{}}},"expressions":{"eps":"abs(a-b)"}})
```

<!-- end -->

<!-- example for float mul -->

Another alternative, which can also be used to perform `IN(attr,val1,val2,val3)` is to compare floats as integers by choosing a multiplier factor and convert the floats to integers in operations. Example illustrates modifying `IN(attr,2.0,2.5,3.5)` to work with integer values.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select in(ceil(attr*100),200,250,350) from products
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->
```http
POST /search
{
  "index": "products",
  "query": { "match_all": {} } },
  "expressions": { "inc": "in(ceil(attr*100),200,250,350)" }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->expression('inc','in(ceil(attr*100),200,250,350)')->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"index":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}})
```
<!-- end -->

## JSON

<!-- example for creating json -->

This data type allows storing JSON objects for schema-less data.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text, data json);
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products(title text, data json)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'data'=>['type'=>'json']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text, data json)')
```

<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_json = data
}
```

<!-- end -->


<!-- example for INDEXOF() json -->

JSON properties can be used in most operations. There are also special functions such as [ALL()](Functions/Arrays_and_conditions_functions.md#ALL%28%29), [ANY()](Functions/Arrays_and_conditions_functions.md#ANY%28%29), [GREATEST()](Functions/Mathematical_functions.md#GREATEST%28%29), [LEAST()](Functions/Mathematical_functions.md#LEAST%28%29) and [INDEXOF()](Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29) that allow traversal of property arrays.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select indexof(x>2 for x in data.intarray) from products
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /search
{
  "index": "products",
  "query": { "match_all": {} } },
  "expressions": { "idx": "indexof(x>2 for x in data.intarray)" }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->expression('idx','indexof(x>2 for x in data.intarray)')->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"index":"products","query":{"match_all":{}}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}})
```

<!-- end -->

<!-- example for REGEX() json -->

Text properties are treated same as strings so it's not possible to use them in full-text matches expressions, but string functions like [REGEX()](Functions/String_functions.md#REGEX%28%29) can be used.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select regex(data.name, 'est') as c from products where c>0
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /search
{
  "index": "products",
  "query":
  {
    "match_all": {},
	"range": { "c": { "gt": 0 } } }
  },
  "expressions": { "c": "regex(data.name, 'est')" }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->expression('idx',"regex(data.name, 'est')")->filter('c','gt',0)->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"index":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}})
```
<!-- end -->

<!-- example for DOUBLE() -->

In case of JSON properties, enforcing data type is required to be casted in some situations for proper functionality. For example in case of float values [DOUBLE()](Functions/Type_casting_functions.md#DOUBLE%28%29) must be used for proper sorting.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products order by double(data.myfloat) desc
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /search
{
  "index": "products",
  "query": { "match_all": {} } },
  "sort": [ { "double(data.myfloat)": { "order": "desc"} } ]
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->sort('double(data.myfloat)','desc')->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"index":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]})
```
<!-- end -->

## Multi-value integer (MVA)

<!-- example for creating MVA32 -->

Multi-value attributes allow storing variable-length lists of 32-bit unsigned integers. It can be used to store one-to-many numeric values like tags, product categories, properties.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text, product_codes multi);
```

<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products(title text, product_codes multi)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'product_codes'=>['type'=>'multi']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text, product_codes multi)')
```


<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_multi = product_codes
}
```

<!-- end -->


<!-- example for any/all MVA -->
It supports filtering and aggregation, but not sorting. Filtering can made of condition that requires at least one element to pass (using [ANY()](Functions/Arrays_and_conditions_functions.md#ANY%28%29)) or all ([ALL()](Functions/Arrays_and_conditions_functions.md#ALL%28%29)).

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products where any(product_codes)=3
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /search
{
  "index": "products",
  "query":
  {
    "match_all": {},
    "equals" : { "any(product_codes)": 3 } }
  }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->filter('any(product_codes)','equals',3)->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"index":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})
```
<!-- end -->

<!-- example for least/greatest MVA -->

Information like [least](Functions/Mathematical_functions.md#LEAST%28%29) or [greatest](Functions/Mathematical_functions.md#GREATEST%28%29) element and length of the list can be extracted. An example shows ordering by the least element of a multi-value attribute.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select least(product_codes) l from products order by l asc
```
<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /search
{
  "index": "products",
  "query":
  {
    "match_all": {},
	"sort": [ { "product_codes":{ "order":"asc", "mode":"min" } } ]
  }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->search('')->sort('product_codes','asc','min')->get();
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
searchApi.search({"index":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}})
```
<!-- end -->

<!-- example for grouping by MVA -->
When grouping by multi-value attribute, a document will contribute to as many groups as there are different values associated with that document. For instance, if the collection contains exactly 1 document having a 'product_codes' multi-value attribute with values 5, 7, and 11, grouping on 'product_codes' will produce 3 groups with `COUNT(*)`equal to 1 and `GROUPBY()` key values of 5, 7, and 11 respectively. Also note that grouping by multi-value attributes might lead to duplicate documents in the result set because each document can participate in many groups.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
insert into products values ( 1, 'doc one', (5,7,11) );
select id, count(*), groupby() from products group by product_codes;
```

<!-- response mysql -->

```sql
Query OK, 1 row affected (0.00 sec)

+------+----------+-----------+
| id   | count(*) | groupby() |
+------+----------+-----------+
|    1 |        1 |        11 |
|    1 |        1 |         7 |
|    1 |        1 |         5 |
+------+----------+-----------+
3 rows in set (0.00 sec)
```

<!-- end -->

<!-- example for MVA value order -->
The order of the numbers inserted as values of multi-valued attributes is not preserved. Values are stored internally as a sorted set.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
insert into product values (1,'first',(4,2,1,3));
select * from products;
```

<!-- response mysql -->

```sql
Query OK, 1 row affected (0.00 sec)

+------+---------------+-------+
| id   | product_codes | title |
+------+---------------+-------+
|    1 | 1,2,3,4       | first |
+------+---------------+-------+
1 row in set (0.01 sec)
```

<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /insert
{
	"index":"products",
	"id":1,
	"doc":
	{
		"title":"first",
		"product_codes":[4,2,1,3]
	}
}

POST /search
{
  "index": "products",
  "query": { "match_all": {} }
}
```

<!-- response HTTP -->

```http
{
   "_index":"products",
   "_id":1,
   "created":true,
   "result":"created",
   "status":201
}

{
   "took":0,
   "timed_out":false,
   "hits":{
      "total":1,
      "hits":[
         {
            "_id":"1",
            "_score":1,
            "_source":{
               "product_codes":[
                  1,
                  2,
                  3,
                  4
               ],
               "title":"first"
            }
         }
      ]
   }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->addDocument([
    "title"=>"first",
    "product_codes"=>[4,2,1,3]
]);
$index->search('')-get();
```

<!-- response PHP -->

```php
Array
(
    [_index] => products
    [_id] => 1
    [created] => 1
    [result] => created
    [status] => 201
)
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] => Array
                                (
                                    [product_codes] => Array
                                        (
                                            [0] => 1
                                            [1] => 2
                                            [2] => 3
                                            [3] => 4
                                        )

                                    [title] => first
                                )
                        )
                )
        )
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
indexApi.insert({"index":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}})
searchApi.search({"index":"products","query":{"match_all":{}}})
```
<!-- response Python -->

```python
{'created': True,
 'found': None,
 'id': 1,
 'index': 'products',
 'result': 'created'}
{'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1,
                    u'_source': {u'product_codes': [1, 2, 3, 4],
                                 u'title': u'first'}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 29}
```

<!-- end -->


## Multi-value big integer

<!-- example for creating MVA64 -->

A data type type that allows storing variable-length lists of 64-bit signed integers. It has the same functionality as multi-value integer.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text, values multi64);
```

<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=create table products(title text, values multi64)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'values'=>['type'=>'multi64']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text, values multi64))')
```

<!-- intro -->
##### config:

<!-- request config -->

```config
index products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_multi_64 = values
}
```

<!-- end -->
