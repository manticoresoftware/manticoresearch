# डेटा प्रकार


## पूर्ण-पाठ फ़ील्ड और विशेषताएँ


Manticore के डेटा प्रकारों को दो श्रेणियों में विभाजित किया जा सकता है: पूर्ण-पाठ फ़ील्ड और विशेषताएँ.


### फ़ील्ड नाम सिंटैक्स


Manticore में फ़ील्ड नामों को इन नियमों का पालन करना चाहिए:


* इसमें अक्षर (a-z, A-Z), नंबर (0-9), और हायफन (-) शामिल हो सकते हैं

* एक अक्षर से शुरू होना चाहिए

* नंबर केवल अक्षरों के बाद ही हो सकते हैं

* अंडरस्कोर (`_`) एकमात्र अनुमत विशेष वर्ण है

* फ़ील्ड नाम केस-संवेदनशील नहीं होते


उदाहरण के लिए:
* मान्य फ़ील्ड नाम: `title`, `product_id`, `user_name_2`
* अमान्य फ़ील्ड नाम: `2title`, `-price`, `user@name`


### पूर्ण-पाठ फ़ील्ड


पूर्ण-पाठ फ़ील्ड:
* प्राकृतिक भाषा प्रसंस्करण एल्गोरिदम के साथ इंडेक्स किए जा सकते हैं, इसलिए कीवर्ड के लिए खोजा जा सकता है
* क्रमबद्ध करने या समूह बनाने के लिए उपयोग नहीं किया जा सकता
* मूल दस्तावेज़ की सामग्री पुनः प्राप्त की जा सकती है
* मूल दस्तावेज़ की सामग्री को हाइलाइटिंग के लिए उपयोग किया जा सकता है


पूर्ण-पाठ फ़ील्ड को डेटा प्रकार `text` द्वारा दर्शाया जाता है. अन्य सभी डेटा प्रकारों को "विशेषताएँ" कहा जाता है.


### विशेषताएँ


विशेषताएँ प्रत्येक दस्तावेज़ से संबंधित गैर-पूर्ण-पाठ मान होते हैं जिन्हें खोज के दौरान गैर-पूर्ण-पाठ फ़िल्टरिंग, क्रमबद्ध करने और समूह बनाने के लिए उपयोग किया जा सकता है.


अक्सर यह चाहा जाता है कि पूर्ण-पाठ खोज परिणामों को न केवल मेल खाते दस्तावेज़ आईडी और उसकी रैंक के आधार पर, बल्कि अन्य कई प्रति-दस्तावेज़ मानों के आधार पर भी प्रोसेस किया जाए. उदाहरण के लिए, हो सकता है कि समाचार खोज परिणामों को तिथि और फिर प्रासंगिकता के अनुसार क्रमबद्ध करना हो, या निर्दिष्ट मूल्य सीमा के भीतर उत्पादों में खोज करनी हो, या एक ब्लॉग खोज को चुने हुए उपयोगकर्ताओं द्वारा बनाए गए पोस्ट तक सीमित करना हो, या परिणामों को महीने के अनुसार समूहित करना हो. इसे कुशलतापूर्वक करने के लिए, Manticore न केवल पूर्ण-पाठ फ़ील्ड को सक्षम करता है, बल्कि प्रत्येक दस्तावेज़ में अतिरिक्त विशेषताएँ भी जोड़ने की अनुमति देता है. इन विशेषताओं का उपयोग पूर्ण-पाठ मेल को फ़िल्टर, क्रमबद्ध या समूहित करने के लिए किया जा सकता है, या केवल विशेषताओं द्वारा खोज करने के लिए किया जा सकता है.


पूर्ण-पाठ फ़ील्ड के विपरीत, विशेषताओं को पूर्ण-पाठ इंडेक्स नहीं किया जाता है. इन्हें टेबल में संग्रहीत किया जाता है, लेकिन इन्हें पूर्ण-पाठ के रूप में खोजा जाना संभव नहीं है.


<!-- example attributes or fields -->


विशेषताओं का एक अच्छा उदाहरण एक फोरम पोस्ट टेबल हो सकता है. मान लीजिए कि केवल शीर्षक और सामग्री फ़ील्ड को पूर्ण-पाठ खोजयोग्य होना आवश्यक है - लेकिन कभी-कभी यह भी आवश्यक होता है कि खोज को किसी विशिष्ट लेखक या उप-फोरम (अर्थात, केवल उन पंक्तियों में खोज करें जिनमें author_id या forum_id के कुछ विशिष्ट मान हों) तक सीमित किया जाए; या मेल को post_date कॉलम द्वारा क्रमबद्ध किया जाए; या मेल खाने वाले पोस्ट्स को post_date के महीने के अनुसार समूहित करके प्रति-समूह मेल की संख्या की गणना की जाए.


<!-- intro -->
##### SQL:
<!-- request SQL -->


```sql
CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp);
```
<!-- intro -->
##### JSON:


<!-- request JSON -->


```JSON
POST /cli -d "CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)"
```


<!-- intro -->
##### PHP:


<!-- request PHP -->


```php
$index = new ManticoresearchIndex($client);
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


```python
utilsApi.sql('CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)')
```


<!-- intro -->
##### Python-asyncio:


<!-- request Python-asyncio -->


```python
await utilsApi.sql('CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)')
```


<!-- intro -->
##### Javascript:


<!-- request Javascript -->


```javascript
res = await utilsApi.sql('CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)');
```


<!-- intro -->
##### Java:


<!-- request Java -->


```java
utilsApi.sql("CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)");
```


<!-- intro -->
##### C#:


<!-- request C# -->


```java
utilsApi.Sql("CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)");
```


<!-- intro -->
##### Rust:


<!-- request Rust -->


```rust
utils_api.sql("CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)", Some(true)).await;
```


<!-- intro -->
##### config:


<!-- request config -->


```ini
table forum
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


यह उदाहरण `author_id`, `forum_id` द्वारा फ़िल्टर किए गए और `post_date` द्वारा क्रमबद्ध पूर्ण-पाठ क्वेरी चलाने का प्रदर्शन करता है.


<!-- intro -->
##### SQL:
<!-- request SQL -->


```sql
select * from forum where author_id=123 and forum_id in (1,3,7) order by post_date desc
```
<!-- intro -->
##### JSON:


<!-- request JSON -->


```JSON
POST /search
{
  "table": "forum",
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
        'table' => 'forum',
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
searchApi.search({"table":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
HashMap<String,Object> filters = new HashMap<String,Object>(){{
    put("must", new HashMap<String,Object>(){{
        put("equals",new HashMap<String,Integer>(){{
            put("author_id",123);
        }});
        put("in",
            new HashMap<String,Object>(){{
                put("forum_id",new int[] {1,3,7});
        }});
    }});
}};
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("bool",filters);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("post_date","desc");}});
}});
SearchResponse searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
var boolFilter = new BoolFilter();
boolFilter.Must = new List<Object> {
    new EqualsFilter("author_id", 123),
    new InFilter("forum_id", new List<Object> {1,3,7})
};
searchRequest.AttrFilter = boolFilter;
searchRequest.Sort = new List<Object> { new SortOrder("post_date", SortOrder.OrderEnum.Desc) };
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut sort = HashMap::new();
sort.insert("post_date".to_string(), serde_json::json!("desc"));
let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    sort: serde_json::json!(sort)
    ..Default::default()
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

### Row-wise and columnar attribute storages

Manticore supports two types of attribute storages:
* row-wise - पारंपरिक स्टोरेज जो Manticore Search में उपलब्ध है
* columnar - [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) द्वारा प्रदान किया गया

As can be understood from their names, they store data differently. The traditional **row-wise storage**:
* बिना संकुचन के गुणों को संग्रहीत करता है
* एक ही दस्तावेज़ के सभी गुण एक पंक्ति में एक-दूसरे के पास संग्रहीत होते हैं
* पंक्तियाँ एक-एक करके संग्रहीत होती हैं
* गुणों तक पहुँच मूल रूप से पंक्ति आईडी को स्ट्राइड (एकल वेक्टर की लंबाई) से गुणा करके और गणना की गई मेमोरी स्थान से अनुरोधित गुण प्राप्त करके किया जाता है। यह बहुत कम रैंडम एक्सेस विलंबता प्रदान करता है।
* गुणों को स्वीकार्य प्रदर्शन प्राप्त करने के लिए मेमोरी में होना चाहिए, अन्यथा संग्रहण की पंक्ति-वार प्रकृति के कारण Manticore को बहुत अधिक अनावश्यक डेटा डिस्क से पढ़ना पड़ सकता है जो कई मामलों में उप-पर्याप्त है।

With **the columnar storage**:
* प्रत्येक गुण को सभी अन्य गुणों से अलग "कॉलम" में स्वतंत्र रूप से संग्रहीत किया जाता है
* संग्रहण 65536 प्रविष्टियों के ब्लॉकों में बाँटा जाता है
* ब्लॉक संकुचित रूप से संग्रहीत होते हैं। यह अक्सर पंक्ति-वार स्टोरेज के समान सभी मूल्यों को संग्रहीत करने के बजाय केवल कुछ विशेष मूल्यों को संग्रहीत करने की अनुमति देता है। उच्च संकुचन अनुपात डिस्क से तेजी से पढ़ने की अनुमति देता है और मेमोरी की आवश्यकता को बहुत कम कर देता है
* जब डेटा को अनुक्रमित किया जाता है, तो प्रत्येक ब्लॉक के लिए संग्रहण योजना स्वतंत्र रूप से चुनी जाती है। उदाहरण के लिए, यदि एक ब्लॉक में सभी मान समान हैं, तो इसे "const" संग्रहण मिलता है और पूरे ब्लॉक के लिए केवल एक मान संग्रहीत किया जाता है। यदि एक ब्लॉक में 256 से कम अद्वितीय मान हैं, तो इसे "table" संग्रहण मिलता है और मूल्यों के लिए तालिका के सूचकों को संग्रहीत करता है
* यदि यह स्पष्ट है कि अनुरोधित मान ब्लॉक में उपस्थित नहीं है, तो ब्लॉक में खोज को जल्दी खारिज किया जा सकता है।

The columnar storage was designed to handle large data volume that does not fit into RAM, so the recommendations are:
* यदि आपके पास आपके सभी गुणों के लिए पर्याप्त मेमोरी है, तो आप पंक्ति-वार संग्रहण से लाभान्वित होंगे
* अन्यथा, कॉलम संग्रहण अभी भी आपको बहुत कम मेमोरी फूटप्रिंट के साथ उचित प्रदर्शन दे सकता है जो आपको अपने तालिका में और अधिक दस्तावेज़ संग्रहीत करने की अनुमति देगा

### How to switch between the storages

The traditional row-wise storage is the default, so if you want everything to be stored in a row-wise fashion, you don't need to do anything when you create a table.

To enable the columnar storage you need to:
* specify `engine='columnar'` in [CREATE TABLE](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Creating-a-real-time-table-online-via-CREATE-TABLE) to make all attributes of the table columnar. Then, if you want to keep a specific attribute row-wise, you need to add `engine='rowwise'` when you declare it. For example:
```sql
create table tbl(title text, type int, price float engine='rowwise') engine='columnar'
```
* एक विशेष विशेषता के लिए `CREATE TABLE` में `engine='columnar'` निर्दिष्ट करें ताकि यह कॉलमर बन जाए। उदाहरण के लिए:
```sql
create table tbl(title text, type int, price float engine='columnar');
```
या
```sql
create table tbl(title text, type int, price float engine='columnar') engine='rowwise';
```
* [साधारण मोड](../Read_this_first.md#Real-time-mode-vs-plain-mode) में, आपको उन विशेषताओं की सूची देनी होगी जिन्हें आप [columnar_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) में कॉलमर बनाना चाहते हैं।


नीचे मंटिकोर सर्च द्वारा समर्थित डेटा प्रकारों की सूची दी गई है:

## दस्तावेज़ आईडी

दस्तावेज़ पहचानकर्ता एक अनिवार्य विशेषता है जो एक अद्वितीय 64-बिट असाइन किया गया पूर्णांक होना चाहिए। दस्तावेज़ आईडी को टेबल बनाते समय स्पष्ट रूप से निर्दिष्ट किया जा सकता है, लेकिन यदि निर्दिष्ट नहीं किया गया हो तो भी वे हमेशा सक्षम होते हैं। दस्तावेज़ आईडी को अपडेट नहीं किया जा सकता है।

जब आप एक तालिका बनाते हैं, तो आप आईडी को स्पष्ट रूप से निर्दिष्ट कर सकते हैं, लेकिन आप जिस भी डेटा प्रकार का उपयोग करते हैं, यह हमेशा उपरोक्त वर्णित तरीके से काम करेगा - असाइन किया गया 64-बिट के रूप में संग्रहीत लेकिन साइन किए गए 64-बिट पूर्णांक के रूप में उजागर किया गया।

```sql
mysql> CREATE TABLE tbl(id bigint, content text);
DESC tbl;
+---------+--------+----------------+
| Field   | Type   | Properties     |
+---------+--------+----------------+
| id      | bigint |                |
| content | text   | indexed stored |
+---------+--------+----------------+
2 rows in set (0.00 sec)
```

आप आईडी को पूरी तरह से निर्दिष्ट करने को छोड़ भी सकते हैं, यह स्वचालित रूप से सक्षम हो जाएगा।
```sql
mysql> CREATE TABLE tbl(content text);
DESC tbl;
+---------+--------+----------------+
| Field   | Type   | Properties     |
+---------+--------+----------------+
| id      | bigint |                |
| content | text   | indexed stored |
+---------+--------+----------------+
2 rows in set (0.00 sec) 
```

दस्तावेज़ आईडी के साथ काम करते समय, यह जानना महत्वपूर्ण है कि इन्हें आंतरिक रूप से असाइन किए गए 64-बिट पूर्णांकों के रूप में संग्रहीत किया जाता है, लेकिन क्वेरी और परिणामों में साइन किए गए 64-बिट पूर्णांकों के रूप में उजागर किया जाता है। इसका मतलब है:

* 2^63-1 से बड़े आईडी नकारात्मक संख्याओं के रूप में दिखाई देंगे।
* ऐसे बड़े आईडी के द्वारा छाँटते समय, आपको उनके साइन किए गए प्रतिनिधित्व का उपयोग करना होगा।
* वास्तविक असाइन किया गया मूल्य देखने के लिए [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) फ़ंक्शन का उपयोग करें।

उदाहरण के लिए, चलिए एक तालिका बनाते हैं और 2^63 के चारों ओर कुछ मान डालते हैं:
```sql
mysql> create table t(id_text string);
Query OK, 0 rows affected (0.01 sec)

mysql> insert into t values(9223372036854775807, '2 ^ 63 - 1'),(9223372036854775808, '2 ^ 63');
Query OK, 2 rows affected (0.00 sec)
```

कुछ आईडी परिणामों में नकारात्मक संख्याओं के रूप में दिखाई देते हैं क्योंकि वे 2^63-1 से अधिक होते हैं। हालांकि, `UINT64(id)` का उपयोग करके उनके वास्तविक असाइन किए गए मान प्रकट हो सकते हैं:
```sql
mysql> select *, uint64(id) from t;
+----------------------+------------+---------------------+
| id                   | id_text    | uint64(id)          |
+----------------------+------------+---------------------+
|  9223372036854775807 | 2 ^ 63 - 1 | 9223372036854775807 |
| -9223372036854775808 | 2 ^ 63     | 9223372036854775808 |
+----------------------+------------+---------------------+
2 rows in set (0.00 sec)
--- 2 out of 2 results in 0ms ---
```

दस्तावेज़ों को 2^63 से कम आईडी के साथ क्वेरी करने के लिए, आप सीधे असाइन किए गए मान का उपयोग कर सकते हैं:
```sql
mysql> select * from t where id = 9223372036854775807;
+---------------------+------------+
| id                  | id_text    |
+---------------------+------------+
| 9223372036854775807 | 2 ^ 63 - 1 |
+---------------------+------------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

हालाँकि, 2^63 से शुरू होने वाले आईडी के लिए, आपको साइन किए गए मान का उपयोग करना होगा:
```sql
mysql> select * from t where id = -9223372036854775808;
+----------------------+---------+
| id                   | id_text |
+----------------------+---------+
| -9223372036854775808 | 2 ^ 63  |
+----------------------+---------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

यदि आप इसके बजाय असाइन किए गए मान का उपयोग करते हैं, तो आपको एक त्रुटि मिलेगी:
```sql
mysql> select * from t where id = 9223372036854775808;
ERROR 1064 (42000): number 9223372036854775808 is out of range [-9223372036854775808..9223372036854775807]
```

जो मान 64 बिट में फिट नहीं होते हैं, वे इसी प्रकार की त्रुटि उत्पन्न करेंगे:

```sql
mysql> select * from t where id = -9223372036854775809;
ERROR 1064 (42000): number -9223372036854775809 is out of range [-9223372036854775808..9223372036854775807]
```

## वर्ण डेटा प्रकार

सामान्य तर्क:
```
string|text [stored|attribute] [indexed]
```

**गुण:**

1. `indexed` - पूर्ण-पाठ सूचकांकित (पूर्ण-पाठ क्वेरी में उपयोग किया जा सकता है)
2. `stored` - एक डॉकस्टोर में संग्रहीत (डिस्क पर संग्रहीत, RAM में नहीं, सुस्त पढ़ाई)
3. `attribute` - इसे एक स्ट्रिंग विशेषता बनाता है (इसके द्वारा क्रमबद्ध/समूहित किया जा सकता है)

कम से कम एक गुण निर्दिष्ट करने से सभी डिफ़ॉल्ट गुणों को ओवरराइड करता है (नीचे देखें), यानी यदि आप गुणों के एक कस्टम संयोजन का उपयोग करने का निर्णय लेते हैं, तो आपको सभी गुणों की सूची बनानी होगी जिन्हें आप चाहते हैं।

**कोई गुण निर्दिष्ट नहीं किया गया:**

`string` और `text` उपनाम हैं, लेकिन यदि आप कोई गुण निर्दिष्ट नहीं करते हैं, तो वे डिफ़ॉल्ट रूप से भिन्न अर्थ रखते हैं:

* केवल `string` डिफ़ॉल्ट रूप से `attribute` का अर्थ है (विवरण देखें [नीचे](../Creating_a_table/Data_types.md#Text))।
* केवल `text` डिफ़ॉल्ट रूप से `stored` + `indexed` का अर्थ है (विवरण देखें [नीचे](../Creating_a_table/Data_types.md#String))।

### टेक्स्ट

<!-- example working with text -->

टेक्स्ट (सिर्फ `text` या `text/string indexed`) डेटा प्रकार तालिका के पूर्ण-पाठ भाग को बनाता है। टेक्स्ट फ़ील्ड सूचकांकित होते हैं और कीवर्ड के लिए खोजा जा सकता है।

टेक्स्ट को एक विश्लेषक पाइपलाइन के माध्यम से पास किया जाता है जो टेक्स्ट को शब्दों में परिवर्तित करता है, रूपांतरणों को लागू करता है, इत्यादि। अंततः, उस टेक्स्ट से एक पूर्ण-पाठ तालिका (एक विशेष डेटा संरचना जो कीवर्ड के लिए त्वरित खोजों की अनुमति देती है) का निर्माण किया जाता है।
पूर्ण-टेक्स्ट फ़ील्ड केवल `MATCH()` क्लॉज में उपयोग किए जा सकते हैं और क्रमबद्धता या संक्षेपण के लिए उपयोग नहीं किए जा सकते हैं। शब्दों को एक उल्टे सूचकांक में संग्रहीत किया जाता है, साथ ही उन फ़ील्डों के संदर्भों के साथ जिनसे वे संबंधित हैं और फ़ील्ड में उनके स्थान। यह प्रत्येक फ़ील्ड के भीतर शब्द की खोज की अनुमति देता है और निकटता जैसी उन्नत ऑपरेटरों का उपयोग करता है। डिफ़ॉल्ट रूप से, फ़ील्ड का मूल पाठ दोनों इंडेक्स और दस्तावेज़ भंडारण में संग्रहीत होता है। इसका मतलब है कि मूल पाठ को क्वेरी परिणामों के साथ वापस किया जा सकता है और [खोज परिणाम हाइलाइटिंग](../Searching/Highlighting.md) में उपयोग किया जा सकता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{

type = rt

path = products


# जब कॉन्फ़िग के माध्यम से फ़ील्ड की कॉन्फ़िगर करते हैं, तो वे डिफ़ॉल्ट रूप से इंडेक्स होते हैं (और संग्रहीत नहीं होते)

rt_field = title


# इस विकल्प को फ़ील्ड को संग्रहीत करने के लिए निर्दिष्ट किया जाना आवश्यक है

stored_fields = title
}
```

<!-- end -->

<!-- indexed केवल के साथ कार्य करने का उदाहरण  -->

इस व्यवहार को स्पष्ट रूप से निर्दिष्ट करके अधिलेखित किया जा सकता है कि पाठ केवल इंडेक्स किया गया है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text indexed);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text indexed)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text','options'=>['indexed']]
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text indexed)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text indexed)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text indexed)');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text indexed)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{

type = rt

path = products


# जब कॉन्फ़िग के माध्यम से फ़ील्ड की कॉन्फ़िगर करते हैं, तो वे डिफ़ॉल्ट रूप से इंडेक्स होते हैं (और संग्रहीत नहीं होते)

rt_field = title
}
```

<!-- end -->

<!-- फ़ील्ड नामकरण के लिए उदाहरण  -->

फ़ील्डों को नामित किया जाता है, और आप अपनी खोजों को एक ही फ़ील्ड (जैसे "title" के माध्यम से केवल खोजें) या फ़ील्ड के एक उपसमुच्चय (जैसे केवल "title" और "abstract") तक सीमित कर सकते हैं। आपके पास 256 तक पूर्ण-टेक्स्ट फ़ील्ड हो सकते हैं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products where match('@title first');
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{

"table": "products",

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
searchApi.search({"table":"products","query":{"match":{"title":"first"}}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match":{"title":"first"}}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match":{"title":"first"}}});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text indexed)", Some(true)).await;
```

<!-- end -->

### स्ट्रिंग

<!-- स्ट्रिंग विशेषताओं के लिए उदाहरण  -->
फुल-टेक्स्ट फ़ील्ड के विपरीत, स्ट्रिंग विशेषताएँ (केवल `string` या `string/text attribute`) उन्हें प्राप्त होने पर जैसे हैं, वैसे ही स्टोर की जाती हैं और इनका उपयोग फुल-टेक्स्ट खोजों में नहीं किया जा सकता। इसके बजाय, इन्हें परिणामों में वापस किया जाता है, तुलना फ़िल्टरिंग या `REGEX` के लिए `WHERE` क्लॉज़ में उपयोग किया जा सकता है, और इन्हें क्रमबद्धता और संचित करने के लिए उपयोग किया जा सकता है। सामान्यत: यह अनुशंसित नहीं है कि बड़ी टेक्स्ट को स्ट्रिंग विशेषताओं में स्टोर किया जाए, बल्कि नामों, शीर्षकों, टैग, कुंजी जैसे मेटाडेटा के लिए स्ट्रिंग विशेषताओं का उपयोग करें।

यदि आप स्ट्रिंग विशेषता को भी अनुक्रमित करना चाहते हैं, तो आप दोनों को `string attribute indexed` के रूप में निर्दिष्ट कर सकते हैं। यह फुल-टेक्स्ट खोज की अनुमति देगा और एक विशेषता के रूप में काम करेगा।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, keys string);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, keys string)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, keys string)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, keys string)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, keys string)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, keys string)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, keys string)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, keys string)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

आप एक फुल-टेक्स्ट फ़ील्ड बना सकते हैं जो एक स्ट्रिंग विशेषता के रूप में भी संग्रहित है। यह दृष्टिकोण एक फुल-टेक्स्ट फ़ील्ड और एक स्ट्रिंग विशेषता उत्पन्न करता है जिनका नाम समान होता है। ध्यान दें कि आप `stored` संपत्ति को जोड़कर डेटा को एक स्ट्रिंग विशेषता और दस्तावेज़ भंडारण में एक ही समय में संग्रहित नहीं कर सकते।

<!-- intro -->
##### SQL:
<!-- request SQL -->
`string attribute indexed` का मतलब है कि हम एक स्ट्रिंग डेटा प्रकार के साथ काम कर रहे हैं जो एक विशेषता के रूप में संग्रहित है और फुल-टेक्स्ट फ़ील्ड के रूप में अनुक्रमित है।

```sql
CREATE TABLE products ( title string attribute indexed );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products ( title string attribute indexed )"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'string','options'=>['indexed','attribute']]
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products ( title string attribute indexed )')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products ( title string attribute indexed )')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products ( title string attribute indexed )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products ( title string attribute indexed )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products ( title string attribute indexed )");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products ( title string attribute indexed )", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{

type = rt

path = products


rt_field = title

rt_attr_string = title
}
```

<!-- end -->

</details>

### Manticore में बाइनरी डेटा संग्रहित करना

<!-- example binary -->

Manticore के पास बाइनरी डेटा के लिए कोई समर्पित फ़ील्ड प्रकार नहीं है, लेकिन आप इसे base64 एन्कोडिंग का उपयोग करके सुरक्षित रूप से संग्रहित कर सकते हैं और `text stored` या `string stored` फ़ील्ड प्रकार (जो पर्यायवाची हैं) का उपयोग कर सकते हैं। यदि आप बाइनरी डेटा को एन्कोड नहीं करते हैं, तो इसका कुछ हिस्सा खो सकता है - उदाहरण के लिए, Manticore एक स्ट्रिंग के अंत को ट्रिम करता है यदि यह शून्य-बाइट का सामना करता है।

यहाँ एक उदाहरण है जहाँ हम `ls` कमांड को base64 का उपयोग करके एन्कोड करते हैं, इसे Manticore में संग्रहित करते हैं, और फिर यह सत्यापित करने के लिए डिकोड करते हैं कि MD5 चेकसम अपरिवर्तित है:

<!-- request Example -->
```bash
# md5sum /bin/ls
43d1b8a7ccda411118e2caba685f4329  /bin/ls
# encoded_data=`base64 -i /bin/ls `
# mysql -P9306 -h0 -e "drop table if exists test; create table test(data text stored); insert into test(data) values('$encoded_data')"
# mysql -P9306 -h0 -NB -e "select data from test" | base64 -d > /tmp/ls | md5sum
43d1b8a7ccda411118e2caba685f4329  -
```
<!-- end -->

## पूर्णांक

<!-- example for integers  -->

पूर्णांक प्रकार 32 बिट **unsigned** पूर्णांक मानों को संग्रहित करने की अनुमति देता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price int);
```
<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, price int)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price int)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price int)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price int)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price int)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price int)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, price int)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

Integers can be stored in shorter sizes than 32-bit by specifying a bit count. For example, if we want to store a numeric value which we know is not going to be bigger than 8, the type can be defined as `bit(3)`. Bitcount integers perform slower than the full-size ones, but they require less RAM. They are saved in 32-bit chunks, so in order to save space, they should be grouped at the end of attribute definitions (otherwise a bitcount integer between 2 full-size integers will occupy 32 bits as well).

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, flags bit(3), tags bit(2) );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, flags bit(3), tags bit(2))"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, flags bit(3), tags bit(2) ')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, flags bit(3), tags bit(2) ')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, flags bit(3), tags bit(2) ');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, flags bit(3), tags bit(2)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, flags bit(3), tags bit(2)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, flags bit(3), tags bit(2)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

Big integers (bigint) are 64-bit wide **signed** integers.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price bigint );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, price bigint)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price bigint )')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price bigint )')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price bigint )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price bigint )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price bigint )");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, price bigint )", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{

type = rt

path = products


rt_field = title

stored_fields = title


rt_attr_bigint = type
}
```

<!-- end -->

## Boolean

<!-- example for boolean  -->
एक बूलियन गुण घोषित करता है। यह 1 बिट गिनती वाले पूर्णांक गुण के बराबर है.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, sold bool );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, sold bool)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],

'sold'=>['type'=>'bool']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, sold bool )')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, sold bool )')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, sold bool )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, sold bool )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, sold bool )");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, sold bool )", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{

type = rt

path = products


rt_field = title

stored_fields = title


rt_attr_bool = sold
}
```

<!-- end -->

## टाइमस्टैम्प

<!-- example for timestamps  -->

टाइमस्टैम्प प्रकार यूनिक्स टाइमस्टैम्प का प्रतिनिधित्व करता है, जिन्हें 32-बिट पूर्णांकों के रूप में संग्रहीत किया जाता है। बेसिक पूर्णांकों के विपरीत, टाइमस्टैम्प प्रकार [समय और तिथि](../Functions/Date_and_time_functions.md) फ़ंक्शनों के उपयोग की अनुमति देता है। स्ट्रिंग मानों से रूपांतरण निम्न नियमों का पालन करता है:

- सीमांककों के बिना संख्याएँ, जिनकी लंबाई कम से कम 10 वर्ण होती है, उन्हें वैसे ही टाइमस्टैम्प में परिवर्तित किया जाता है।
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`
- `%Y-%m-%d`
- `%Y-%m`
- `%Y`

इन रूपांतरण विशिष्टों के अर्थ [strptime मैनुअल](https://man7.org/linux/man-pages/man3/strptime.3.html) में विस्तार से दिए गए हैं, `%E*S` को छोड़कर, जिसका अर्थ मिलिसेकंड होता है।

ध्यान दें कि साधारण तालिकाओं में टाइमस्टैम्प का ऑटो-रूपांतरण समर्थित नहीं है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, date timestamp);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, date timestamp)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, date timestamp)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, date timestamp)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, date timestamp)');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, date timestamp)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, date timestamp)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, date timestamp)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{

type = rt

path = products


rt_field = title

stored_fields = title


rt_attr_timestamp = date
}
```

<!-- end -->

## फ़्लोट

<!-- example for float -->
वास्तविक संख्याएँ 32-बिट IEEE 754 सिंगल प्रिसिजन फ्लोट्स के रूप में संग्रहीत की जाती हैं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, coeff float);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, coeff float)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, coeff float)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, coeff float)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, coeff float)');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, coeff float)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, coeff float)");
```

<!-- intro -->
##### रस्ट:

<!-- request Rust -->

``` rust
utils_api.sql("CREATE TABLE products(title text, coeff float)", Some(true)).await;
```

<!-- intro -->
##### कॉन्फ़िग:

<!-- request config -->

```ini
table products
{

type = rt

path = products


rt_field = title

stored_fields = title


rt_attr_float = coeff
}
```

<!-- end -->

<!-- eps की तुलना के लिए उदाहरण -->

पूर्णांक प्रकारों की तुलना करने के विपरीत, दो फ्लोटिंग-पॉइंट संख्याओं की समानता की तुलना करना संभावित राउंडिंग त्रुटियों के कारण अनुशंसित नहीं है। एक अधिक विश्वसनीय दृष्टिकोण एक निकट-समान तुलना का उपयोग करना है, जो कि पूर्ण त्रुटि मार्जिन की जांच करके किया जाता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select abs(a-b)<=0.00001 from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->
```JSON
POST /search
{
  "table": "products",
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
##### पायथन:

<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"eps":"abs(a-b)"}})
```

<!-- intro -->
##### पायथन-एसिंक्रोनस:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"eps":"abs(a-b)"}})
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"eps":"abs(a-b)"}});
```
<!-- intro -->
##### जावा:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("ebs","abs(a-b)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object>{
    new Dictionary<string, string> { {"ebs", "abs(a-b)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### रस्ट:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut expr = HashMap::new(); 
expr.insert("ebs".to_string(), serde_json::json!("abs(a-b)"));
let expressions: [HashMap; 1] = [expr];

let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    expressions: serde_json::json!(expressions),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

<!-- फल गुणा के लिए उदाहरण -->

एक और विकल्प, जिसे `IN(attr,val1,val2,val3)` करने के लिए भी उपयोग किया जा सकता है, फ्लोट्स की तुलना पूर्णांकों के रूप में करने के लिए है, एक गुणक कारक चुनकर और ऑपरेशनों में फ्लोट्स को पूर्णांकों में परिवर्तित करना। निम्नलिखित उदाहरण `IN(attr,2.0,2.5,3.5)` को पूर्णांक मानों के साथ काम करने के लिए संशोधित करने को दर्शाता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select in(ceil(attr*100),200,250,350) from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->
```JSON
POST /search
{
  "table": "products",
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
##### पायथन:

<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}})
```

<!-- intro -->
##### पायथन-एसिंक्रोनस:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}})
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}});
```

<!-- intro -->
##### जावा:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("inc","in(ceil(attr*100),200,250,350)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"ebs", "in(ceil(attr*100),200,250,350)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### रस्ट:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut expr = HashMap::new(); 
expr.insert("ebs".to_string(), serde_json::json!("in(ceil(attr*100),200,250,350)"));
let expressions: [HashMap; 1] = [expr];

let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    expressions: serde_json::json!(expressions),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

<!-- example float_accuracy -->
मंटिकोर में फ्लोट मानों को सटीकता के साथ प्रदर्शित किया जाता है ताकि वे सहेजे गए सटीक मान को दर्शाएं। इस दृष्टिकोण को सटीकता खोने से रोकने के लिए पेश किया गया था, विशेष रूप से ऐसे मामलों के लिए जैसे भौगोलिक समन्वय, जहां 6 दशमलव स्थानों तक गोल करने से गलतियाँ होती हैं।

अब, मंटिकोर पहले 6 अंकों के साथ एक संख्या आउटपुट करता है, फिर इसे मूल मान के साथ पार्स और तुलना करता है। यदि वे मेल नहीं खाते हैं, तो अतिरिक्त अंक जोड़े जाते हैं जब तक कि वे मेल नहीं खाते।

उदाहरण के लिए, यदि कोई फ्लोट मान `19.45` के रूप में डाला गया था, तो मंटिकोर इसे `19.450001` के रूप में प्रदर्शित करेगा ताकि सहेजे गए मान को सटीकता से दर्शाया जा सके।

<!-- request Example -->
```sql
insert into t(id, f) values(1, 19.45)
--------------

Query OK, 1 row affected (0.02 sec)

--------------
select * from t
--------------

+------+-----------+
| id   | f         |
+------+-----------+
|    1 | 19.450001 |
+------+-----------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

<!-- end -->


## JSON

<!-- example for creating json -->

यह डेटा प्रकार JSON ऑब्जेक्ट्स के भंडारण की अनुमति देता है, जो स्कीमा-लेस डेटा को संभालने के लिए विशेष रूप से उपयोगी है। JSON मानों को परिभाषित करते समय, सुनिश्चित करें कि ऑब्जेक्ट्स के लिए खोलने और बंद करने वाले कर्ली ब्रेसेस `{` और `}` शामिल हैं, या एरेज़ के लिए वर्ग ब्रेकेट `[` और `]` शामिल हैं। जबकि JSON के लिए कॉलमर स्टोरेज का समर्थन नहीं है, इसे पारंपरिक पंक्ति-वार भंडारण में संग्रहीत किया जा सकता है। यह ध्यान देने योग्य है कि दोनों संग्रहण प्रकारों को एक ही तालिका के भीतर जोड़ा जा सकता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, data json);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, data json)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, data json)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, data json)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, data json)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, data json)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, data json)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, data json)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

JSON गुणों का उपयोग अधिकांश संचालन में किया जा सकता है। ऐसे विशेष फ़ंक्शंस जैसे [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29), [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29), [GREATEST()](../Functions/Mathematical_functions.md#GREATEST%28%29), [LEAST()](../Functions/Mathematical_functions.md#LEAST%28%29) और [INDEXOF()](../Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29) हैं जो प्रॉपर्टी एरेज़ के पार करने की अनुमति देते हैं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select indexof(x>2 for x in data.intarray) from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("idx","indexof(x>2 for x in data.intarray)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"idx", "indexof(x>2 for x in data.intarray)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut expr = HashMap::new(); 
expr.insert("idx".to_string(), serde_json::json!("indexof(x>2 for x in data.intarray)"));
let expressions: [HashMap; 1] = [expr];

let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    expressions: serde_json::json!(expressions),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```


<!-- end -->

<!-- example for REGEX() json -->

टेक्स्ट गुणों को स्ट्रिंग के समान माना जाता है, इसलिए उन्हें पूर्ण-पाठ मिलान अभिव्यक्तियों में उपयोग करना संभव नहीं है। हालाँकि, [REGEX()](../Functions/String_functions.md#REGEX%28%29) जैसी स्ट्रिंग कार्यों का प्रयोग किया जा सकता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select regex(data.name, 'est') as c from products where c>0
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("range", new HashMap<String,Object>(){{
    put("c", new HashMap<String,Object>(){{
        put("gt",0);
    }});
}});
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("idx","indexof(x>2 for x in data.intarray)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
var rangeFilter = new RangeFilter("c");
rangeFilter.Gt = 0;
searchRequest.AttrFilter = rangeFilter;
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"idx", "indexof(x>2 for x in data.intarray)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut expr = HashMap::new(); 
expr.insert("idx".to_string(), serde_json::json!("indexof(x>2 for x in data.intarray)"));
let expressions: [HashMap; 1] = [expr];

let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    expressions: serde_json::json!(expressions),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```


<!-- end -->

<!-- example for DOUBLE() -->

JSON गुणों के मामले में, कुछ परिस्थितियों में उचित कार्यक्षमता के लिए डेटा प्रकार को लागू करना आवश्यक हो सकता है। उदाहरण के लिए, फ्लोट मानों के साथ काम करते समय, [DOUBLE()](../Functions/Type_casting_functions.md#DOUBLE%28%29) का उपयोग उचित क्रमबद्धता के लिए किया जाना चाहिए।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products order by double(data.myfloat) desc
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("double(data.myfloat)",new HashMap<String,String>(){{ put("order","desc");}});}});
}});
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Sort = new List<Object> {
    new SortOrder("double(data.myfloat)", SortOrder.OrderEnum.Desc)
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut sort = HashMap::new();
sort.insert("double(data.myfloat)".to_string(), serde_json::json!("desc"));
let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    sort: serde_json::json!(sort)
    ..Default::default()
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

## Float vector

<!-- example for creating float_vector -->
Float vector attributes allow storing variable-length lists of floats, primarily used for machine learning applications and similarity searches. This type differs from multi-valued attributes (MVAs) in several important ways:
- Preserves the exact order of values (unlike MVAs which may reorder)
- Retains duplicate values (unlike MVAs which deduplicate)
- No additional processing during insertion (unlike MVAs which sort and deduplicate)

### Usage and Limitations
- Currently only supported in real-time tables
- Can only be utilized in KNN (k-nearest neighbor) searches
- Not supported in plain tables or other functions/expressions
- When used with KNN settings, you cannot `UPDATE` `float_vector` values. Use `REPLACE` instead
- When used without KNN settings, you can `UPDATE` `float_vector` values
- Float vectors cannot be used in regular filters or sorting
- The only way to filter by `float_vector` values is through vector search operations (KNN)

### Common Use Cases
- Image embeddings for similarity search
- Text embeddings for semantic search
- Feature vectors for machine learning
- Recommendation system vectors

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, image_vector float_vector);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, image_vector float_vector)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],

'image_vector'=>['type'=>'float_vector']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, image_vector float_vector)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, image_vector float_vector)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, image_vector float_vector)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, image_vector float_vector)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, image_vector float_vector)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, image_vector float_vector)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{

type = rt

path = products


rt_field = title

stored_fields = title


rt_attr_float_vector = image_vector
}
```

<!-- end -->

For information about using float vectors in searches, see [KNN search](../Searching/KNN.md).

## Multi-value integer (MVA)

<!-- example for creating MVA32 -->

Multi-value attributes allow storing variable-length lists of 32-bit unsigned integers. This can be useful for storing one-to-many numeric values, such as tags, product categories, and properties.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, product_codes multi);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, product_codes multi)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, product_codes multi)')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, product_codes multi)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, product_codes multi)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, product_codes multi)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, product_codes multi)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, product_codes multi)", Some(true)).await;
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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
इसमें छानने और संग्रहण का समर्थन है, लेकिन क्रमबद्धता का नहीं। छानना एक शर्त का उपयोग करके किया जा सकता है जो कम से कम एक तत्व को पास करने की आवश्यकता होती है (जिसका उपयोग करके [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)) या सभी तत्वों ([ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29)) को पास करना।


<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products where any(product_codes)=3
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
  "query":
  {
    "match_all": {},
    "equals" : { "any(product_codes)": 3 }
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
searchApi.search({"table":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})'
```
<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("equals",new HashMap<String,Integer>(){{
     put("any(product_codes)",3);
}});
searchRequest.setQuery(query);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.AttrFilter = new EqualsFilter("any(product_codes)", 3);
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let search_request = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

<!-- example for least/greatest MVA -->

जानकारी जैसे कि [least](../Functions/Mathematical_functions.md#LEAST%28%29) या [greatest](../Functions/Mathematical_functions.md#GREATEST%28%29) तत्व और सूची की लंबाई निकाली जा सकती है। एक उदाहरण एक मल्टी-वैल्यू एट्रिब्यूट के सबसे छोटे तत्व द्वारा क्रमबद्धता दिखाता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select least(product_codes) l from products order by l asc
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("product_codes",new HashMap<String,String>(){{ put("order","asc");put("mode","min");}});}});
}});
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Sort = new List<Object> {
    new SortMVA("product_codes", SortOrder.OrderEnum.Asc, SortMVA.ModeEnum.Min)
};
searchResponse = searchApi.Search(searchRequest);
```

<!-- request Rust -->

``` rust
let query = SearchQuery::new();
let mut sort_opts = HashMap::new(); 
sort_opts.insert("order".to_string(), serde_json::json!("asc"));
sort_opts.insert("mode".to_string(), serde_json::json!("min"));
sort_expr.insert("product_codes".to_string(), serde_json::json!(sort_opts));
let sort: [HashMap; 1] = [sort_expr];

let search_req = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    sort: serde_json::json!(sort),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- end -->

<!-- example for grouping by MVA -->
जब एक बहु-मूल्य विशेषता के द्वारा समूह बनाया जाता है, तो एक दस्तावेज़ उन समूहों में योगदान देगा जितने अलग-अलग मूल्य उस दस्तावेज़ से जुड़े होते हैं। उदाहरण के लिए, यदि एक संग्रह में एक ही दस्तावेज़ है जिसमें 'product_codes' बहु-मूल्य विशेषता है और इसके मूल्य 5, 7, और 11 हैं, तो 'product_codes' पर समूह बनाने से 3 समूह उत्पन्न होंगे जिनका `COUNT(*)` 1 होगा और `GROUPBY()` कुंजी मान 5, 7, और 11 होंगे। इसके अलावा, ध्यान दें कि बहु-मूल्य विशेषताओं के द्वारा समूह बनाने से परिणाम सेट में दोहराए जाने वाले दस्तावेज़ हो सकते हैं क्योंकि प्रत्येक दस्तावेज़ कई समूहों में भाग ले सकता है।

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
बहु-मूल्य विशेषताओं के मान के रूप में डाले गए नंबरों का क्रम संरक्षित नहीं किया गया है। मान आंतरिक रूप से क्रमबद्ध सेट के रूप में संग्रहीत होते हैं।

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
##### JSON:

<!-- request JSON -->

```JSON
POST /insert
{

"table":"products",

"id":1,

"doc":

{

"title":"first",

"product_codes":[4,2,1,3]

}
}

POST /search
{
  "table": "products",
  "query": { "match_all": {} }
}
```

<!-- response JSON -->

```JSON
{
   "table":"products",
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
            "_id": 1,
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
indexApi.insert({"table":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}})
searchApi.search({"table":"products","query":{"match_all":{}}})
```
<!-- response Python -->

```python
{'created': True,
 'found': None,
 'id': 1,
 'table': 'products',
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

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await indexApi.insert({"table":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}})
await searchApi.search({"table":"products","query":{"match_all":{}}})
```
<!-- response Python-asyncio -->

```python
{'created': True,
 'found': None,
 'id': 1,
 'table': 'products',
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

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
await indexApi.insert({"table":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}});
res = await searchApi.search({"table":"products","query":{"match_all":{}}});
```
<!-- response javascript -->

```javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1,"_source":{"product_codes":[1,2,3,4],"title":"first"}}]}}
```
<!-- intro -->
##### java:

<!-- request java -->

```java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","first");
    put("product_codes",new int[] {4,2,1,3});
}};
newdoc.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("products");
searchRequest.setQuery(query);
SearchResponse searchResponse = searchApi.search(searchRequest);
System.out.println(searchResponse.toString() );
```
<!-- response java -->

```java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=1, _score=1, _source={product_codes=[1, 2, 3, 4], title=first}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "first");
doc.Add("product_codes", new List<Object> {4,2,1,3});
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
object query =  new { match_all=null };
var searchRequest = new SearchRequest("products", query);
var searchResponse = searchApi.Search(searchRequest);
Console.WriteLine(searchResponse.ToString())
```
<!-- response C# -->

```clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=1, _score=1, _source={product_codes=[1, 2, 3, 4], title=first}}]
        aggregations: null
    }
    profile: null
}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("first"));
doc.insert("product_codes".to_string(), serde_json::json!([4,2,1,3]));
let insert_req = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc));
let insert_res = index_api.insert(insert_req).await;

let query = SearchQuery::new();
let search_req = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
println!("{:?}", search_res);
```

<!-- response Rust -->

```rust
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=1, _score=1, _source={product_codes=[1, 2, 3, 4], title=first}}]
        aggregations: null
    }
    profile: null
}
```

<!-- end -->


## Multi-value big integer

<!-- example for creating MVA64 -->

A data type that allows storing variable-length lists of 64-bit signed integers. It has the same functionality as multi-value integer.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, values multi64);
```

<!-- intro -->
# डेटा प्रकार

## फुल-टेक्स्ट फ़ील्ड और विशेषताएँ

मंटिकोरे के डेटा प्रकारों को दो श्रेणियों में बांटा जा सकता है: फुल-टेक्स्ट फ़ील्ड और विशेषताएँ।

### फ़ील्ड नाम का सिंटैक्स

मंटिकोरे में फ़ील्ड नामों को इन नियमों का पालन करना चाहिए:

* इसमें अक्षर (a-z, A-Z), संख्या (0-9), और हाइफ़न (-) शामिल हो सकते हैं
* यह एक अक्षर से शुरू होना चाहिए
* संख्या केवल अक्षरों के बाद ही आ सकती है
* अंडरस्कोर (`_`) एकमात्र अनुमत विशेष वर्ण है
* फ़ील्ड नाम केस-सेंसिटिव नहीं होते हैं

उदाहरण के लिए:
* मान्य फ़ील्ड नाम: `title`, `product_id`, `user_name_2`
* अमान्य फ़ील्ड नाम: `2title`, `-price`, `user@name`

### फुल-टेक्स्ट फ़ील्ड

फुल-टेक्स्ट फ़ील्ड:
* प्राकृतिक भाषा प्रसंस्करण एल्गोरिदम के साथ अनुक्रमित किए जा सकते हैं, इसलिए कीवर्डों के लिए खोजा जा सकता है
* इसका उपयोग छांटने या समूह बनाने के लिए नहीं किया जा सकता
* मूल दस्तावेज़ की सामग्री पुनर्प्राप्त की जा सकती है
* मूल दस्तावेज़ की सामग्री को हाइलाइटिंग के लिए उपयोग किया जा सकता है

फुल-टेक्स्ट फ़ील्ड का प्रतिनिधित्व `text` डेटा प्रकार द्वारा किया जाता है। सभी अन्य डेटा प्रकारों को "विशेषताएँ" कहा जाता है।

### विशेषताएँ

विशेषताएँ गैर-फुल-टेक्स्ट मान हैं जो प्रत्येक दस्तावेज़ के साथ जुड़ी होती हैं और जिनका उपयोग खोज के दौरान गैर-फुल-टेक्स्ट फ़िल्टरिंग, छंटाई और समूह बनाने के लिए किया जा सकता है।

अक्सर, यह आवश्यक होता है कि फुल-टेक्स्ट खोज परिणामों को न केवल मेल खाते दस्तावेज़ ID और इसकी रैंक के आधार पर बल्कि अन्य कई प्रति-दस्तावेज़ मानों के आधार पर भी संसाधित किया जाए। उदाहरण के लिए, किसी को समाचार खोज परिणामों को दिनांक के आधार पर छांटने की आवश्यकता हो सकती है और फिर प्रासंगिकता के अनुसार, या निर्दिष्ट मूल्य सीमा के भीतर उत्पादों की खोज करने की आवश्यकता हो सकती है, या चुने गए उपयोगकर्ताओं द्वारा बनाई गई पोस्टों तक ब्लॉग खोज को सीमित करने की आवश्यकता हो सकती है, या परिणामों को महीने के अनुसार समूहित करने की आवश्यकता हो सकती है। ऐसा करने के लिए मंटिकोरे न केवल फुल-टेक्स्ट फ़ील्ड को सक्षम करता है, बल्कि प्रत्येक दस्तावेज़ में जोड़ी जाने वाली अतिरिक्त विशेषताओं को भी सक्षम करता है। इन विशेषताओं का उपयोग फुल-टेक्स्ट मेलों को फ़िल्टर करने, छांटने या समूहित करने के लिए किया जा सकता है, या केवल विशेषताओं द्वारा खोज करने के लिए किया जा सकता है।

विशेषताएँ, फुल-टेक्स्ट फ़ील्ड के विपरीत, फुल-टेक्स्ट अनुक्रमित नहीं होती हैं। इन्हें तालिका में संग्रहीत किया जाता है, लेकिन इन्हें फुल-टेक्स्ट के रूप में खोजना संभव नहीं है।

<!-- उदाहरण विशेषताएँ या फ़ील्ड -->

विशेषताओं के लिए एक अच्छा उदाहरण एक फ़ोरम पोस्ट्स टेबल होगा। मान लीजिए कि केवल शीर्षक और सामग्री फ़ील्ड को फुल-टेक्स्ट खोजने योग्य होना चाहिए - लेकिन कभी-कभी यह आवश्यक होता है कि खोज को किसी विशेष लेखक या उप-फ़ोरम तक सीमित किया जाए (अर्थात, केवल उन पंक्तियों की खोज करें जिनमें लेखक_id या फ़ोरम_id के कुछ विशेष मान हैं); या पोस्ट_date कॉलम के द्वारा मेलों को छांटें; या मेल खाते पोस्टों को पोस्ट_date के महीने के अनुसार समूहित करें और प्रति-समूह मेल की गणना करें।

<!-- परिचय -->
##### SQL:
<!-- अनुरोध SQL -->

```sql
CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp);
```
<!-- परिचय -->
##### JSON:

<!-- अनुरोध JSON -->

```JSON
POST /cli -d "CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)"
```

<!-- परिचय -->
##### PHP:

<!-- अनुरोध PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('forum');
$index->create([
    'title'=>['type'=>'text'],
	'content'=>['type'=>'text'],
	'author_id'=>['type'=>'int'],
	'forum_id'=>['type'=>'int'],
	'post_date'=>['type'=>'timestamp']
]);
```
<!-- परिचय -->
##### Python:

<!-- अनुरोध Python -->

```python
utilsApi.sql('CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)')
```
<!-- परिचय -->
##### Javascript:

<!-- अनुरोध Javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)');
```

<!-- परिचय -->
##### Java:

<!-- अनुरोध Java -->

```java
utilsApi.sql("CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)");

```

<!-- परिचय -->
##### C#:

<!-- अनुरोध C# -->

```java
utilsApi.Sql("CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)");

```

<!-- परिचय -->
##### config:

<!-- अनुरोध config -->

```ini
table forum
{
	type = rt
	path = forum

	# जब कॉन्फ़िग के माध्यम से फ़ील्ड कॉन्फ़िगर करते हैं, तो वे अनुक्रमित होते हैं (और संग्रहीत नहीं होते हैं) डिफ़ॉल्ट रूप से
	rt_field = title
	rt_field = content

	# इस विकल्प को फ़ील्ड को संग्रहीत करने के लिए निर्दिष्ट करने की आवश्यकता है
	stored_fields = title, content

	rt_attr_uint = author_id
	rt_attr_uint = forum_id
	rt_attr_timestamp = post_date
}
```

<!-- अंत -->

<!-- उदाहरण फ़िल्टर की गई क्वेरी -->

यह उदाहरण `author_id`, `forum_id` द्वारा फ़िल्टर की गई फुल-टेक्स्ट क्वेरी चलाने को दर्शाता है और `post_date` द्वारा छांटी गई है।

<!-- परिचय -->
##### SQL:
<!-- अनुरोध SQL -->

```sql
select * from forum where author_id=123 and forum_id in (1,3,7) order by post_date desc
```
<!-- परिचय -->
##### JSON:

<!-- अनुरोध JSON -->

```JSON
POST /search
{
  "table": "forum",
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

<!-- परिचय -->
##### PHP:

<!-- अनुरोध PHP -->

```php
$client->search([
        'table' => 'forum',
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


<!-- परिचय -->
##### Python:

<!-- अनुरोध Python -->

```python
searchApi.search({"table":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
HashMap<String,Object> filters = new HashMap<String,Object>(){{
    put("must", new HashMap<String,Object>(){{
        put("equals",new HashMap<String,Integer>(){{
            put("author_id",123);
        }});
        put("in",
            new HashMap<String,Object>(){{
                put("forum_id",new int[] {1,3,7});
        }});
    }});
}};
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("bool",filters);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("post_date","desc");}});
}});
SearchResponse searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
var boolFilter = new BoolFilter();
boolFilter.Must = new List<Object> {
    new EqualsFilter("author_id", 123),
    new InFilter("forum_id", new List<Object> {1,3,7})
};
searchRequest.AttrFilter = boolFilter;
searchRequest.Sort = new List<Object> { new SortOrder("post_date", SortOrder.OrderEnum.Desc) };
var searchResponse = searchApi.Search(searchRequest);
```

<!-- end -->

### रो-वार और स्तंभीय विशेषता भंडारण

Manticore दो प्रकार के विशेषता भंडारण का समर्थन करता है:
* रो-वार - Manticore Search में आउट ऑफ द बॉक्स उपलब्ध पारंपरिक भंडारण
* स्तंभीय - [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) द्वारा प्रदान किया गया

जैसा कि उनके नाम से समझा जा सकता है, वे डेटा को अलग तरीके से संग्रहीत करते हैं। पारंपरिक **रो-वार भंडारण**:
* विशेषताओं को बिना संकुचित किए संग्रहीत करता है
* एक ही दस्तावेज़ की सभी विशेषताएँ एक पंक्ति में एक दूसरे के निकट संग्रहीत होती हैं
* पंक्तियाँ एक-एक करके संग्रहीत की जाती हैं
* विशेषताओं तक पहुँच मूलतः केवल पंक्ति आईडी को स्ट्राइड (एकल वेक्टर की लंबाई) से गुणा करके और गणना की गई मेमोरी स्थान से अनुरोधित विशेषता प्राप्त करते समय की जाती है। यह बहुत कम रैंडम एक्सेस लेटेंसी देता है।
* विशेषताओं को स्वीकार्य प्रदर्शन प्राप्त करने के लिए मेमोरी में होना चाहिए, अन्यथा भंडारण की रो-वार प्रकृति के कारण Manticore को डिस्क से बहुत अधिक अज्ञात डेटा पढ़ना पड़ सकता है जो कई मामलों में अप्रभावी होता है।

**स्तंभीय भंडारण** के साथ:
* प्रत्येक विशेषता सभी अन्य विशेषताओं से स्वतंत्र रूप से अपने अलग "स्तंभ" में संग्रहीत होती है
* भंडारण 65536 प्रविष्टियों के ब्लॉकों में विभाजित होता है
* ब्लॉकों को संकुचित रूप से संग्रहीत किया जाता है। यह अक्सर रो-वार भंडारण की तरह सभी को संग्रहीत करने के बजाय केवल कुछ विशिष्ट मूल्यों को संग्रहीत करने की अनुमति देता है। उच्च संकुचन अनुपात डिस्क से तेजी से पढ़ने की अनुमति देता है और मेमोरी की आवश्यकता को काफी कम करता है
* जब डेटा को अनुक्रमित किया जाता है, तो प्रत्येक ब्लॉक के लिए भंडारण योजना स्वतंत्र रूप से चुनी जाती है। उदाहरण के लिए, यदि एक ब्लॉक में सभी मान समान हैं, तो इसे "const" भंडारण मिलता है और पूरे ब्लॉक के लिए केवल एक मान संग्रहीत किया जाता है। यदि एक ब्लॉक में 256 से कम अद्वितीय मान हैं, तो इसे "table" भंडारण मिलता है और मानों की टेबल के लिए इंडेक्स संग्रहीत करता है बजाय स्वयं मूल्यों के
* यदि यह स्पष्ट है कि अनुरोधित मान ब्लॉक में उपस्थित नहीं है, तो ब्लॉक में खोज को जल्दी खारिज किया जा सकता है।

स्तंभीय भंडारण को बड़े डेटा वॉल्यूम को संभालने के लिए डिज़ाइन किया गया था जो RAM में नही समाता, इसलिए सिफारिशें हैं:
* यदि आपके पास अपनी सभी विशेषताओं के लिए पर्याप्त मेमोरी है तो आप रो-वार भंडारण से लाभान्वित होंगे
* अन्यथा, स्तंभीय भंडारण अभी भी आपको काफी कम मेमोरी फुटप्रिंट के साथ उचित प्रदर्शन दे सकता है जो आपको अपने तालिका में अधिक दस्तावेज़ संग्रहित करने की अनुमति देगा

### भंडारण के बीच स्विच कैसे करें

पारंपरिक रो-वार भंडारण डिफ़ॉल्ट है, इसलिए यदि आप चाहते हैं कि सब कुछ रो-वार अंदाज में संग्रहीत किया जाए, तो आपको तालिका बनाते समय कुछ भी करने की आवश्यकता नहीं है।

स्तंभीय भंडारण सक्षम करने के लिए, आपको करना होगा:
* सभी तालिका की विशेषताओं को स्तंभीय बनाने के लिए [CREATE TABLE](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Creating-a-real-time-table-online-via-CREATE-TABLE) में `engine='columnar'` निर्दिष्ट करें। फिर, यदि आप एक विशेष विशेषता को रो-वार रखना चाहते हैं, तो आपको इसे घोषित करते समय `engine='rowwise'` जोड़ना होगा। उदाहरण के लिए:
```sql
create table tbl(title text, type int, price float engine='rowwise') engine='columnar'
```
* `CREATE TABLE` में एक विशिष्ट विशेषता के लिए `engine='columnar'` निर्दिष्ट करें ताकि इसे स्तंभीय बनाया जा सके। उदाहरण के लिए:
```sql
create table tbl(title text, type int, price float engine='columnar');
```
या
```sql
create table tbl(title text, type int, price float engine='columnar') engine='rowwise';
```
* [plain mode](../Read_this_first.md#Real-time-mode-vs-plain-mode) में आपको [columnar_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) में स्तंभीय बनाना चाहते हैं उनकी विशेषताओं को सूचीबद्ध करना होगा।


नीचे Manticore Search द्वारा समर्थित डेटा प्रकारों की सूची है:

## दस्तावेज़ आईडी

दस्तावेज़ पहचानकर्ता एक अनिवार्य विशेषता है जो एक अद्वितीय 64-बिट बिना चिह्नित पूर्णांक होनी चाहिए। दस्तावेज़ आईडी को तालिका बनाने के समय स्पष्ट रूप से निर्दिष्ट किया जा सकता है, लेकिन वे हमेशा सक्षम होते हैं, भले ही निर्दिष्ट न किए जाएं। दस्तावेज़ आईडी को अपडेट नहीं किया जा सकता है।

जब आप एक तालिका बनाते हैं, तो आप ID को स्पष्ट रूप से निर्दिष्ट कर सकते हैं, लेकिन आप जिस डेटा प्रकार का उपयोग करते हैं, उसके बावजूद, यह हमेशा ऊपर वर्णित के रूप में कार्य करेगा - बिना चिह्नित 64-बिट के रूप में संग्रहीत किया जाता है लेकिन चिह्नित 64-बिट पूर्णांक के रूप में प्रदर्शित किया जाता है।

```sql
mysql> CREATE TABLE tbl(id bigint, content text);
DESC tbl;
+---------+--------+----------------+
| Field   | Type   | Properties     |
+---------+--------+----------------+
| id      | bigint |                |
| content | text   | indexed stored |
+---------+--------+----------------+
2 rows in set (0.00 sec)
```

आप आईडी को बिल्कुल भी निर्दिष्ट नहीं कर सकते, इसे स्वचालित रूप से सक्षम किया जाएगा।
```sql
mysql> CREATE TABLE tbl(content text);
DESC tbl;
+---------+--------+----------------+
| Field   | Type   | Properties     |
+---------+--------+----------------+
| id      | bigint |                |
| content | text   | indexed stored |
+---------+--------+----------------+
2 rows in set (0.00 sec) 
```

दस्तावेज़ आईडी के साथ काम करते समय, यह जानना महत्वपूर्ण है कि उन्हें आंतरिक रूप से असाइन किये गए 64-बिट पूर्णांक के रूप में संग्रहित किया गया है लेकिन उन्हें प्रश्नों और परिणामों में साइन किए गए 64-बिट पूर्णांक के रूप में प्रदर्शित किया जाता है। इसका अर्थ है:

* आईडी जो 2^63-1 से बड़ी हैं, वे नकारात्मक संख्या के रूप में दिखाई देंगी।
* जब ऐसे बड़े आईडी के द्वारा फ़िल्टर करते हैं, तो आपको उनके साइन किए गए प्रतिनिधित्व का उपयोग करना होगा।
* वास्तविक असाइन किया गया मान देखने के लिए [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) फ़ंक्शन का उपयोग करें।

उदाहरण के लिए, चलिए एक तालिका बनाते हैं और 2^63 के आसपास कुछ मान डालते हैं:
```sql
mysql> create table t(id_text string);
Query OK, 0 rows affected (0.01 sec)

mysql> insert into t values(9223372036854775807, '2 ^ 63 - 1'),(9223372036854775808, '2 ^ 63');
Query OK, 2 rows affected (0.00 sec)
```

कुछ आईडी परिणामों में नकारात्मक संख्या के रूप में दिखाई देती हैं क्योंकि वे 2^63-1 से अधिक होती हैं। हालाँकि, `UINT64(id)` का उपयोग करने से उनके वास्तविक असाइन किए गए मान प्रकट हो सकते हैं:
```sql
mysql> select *, uint64(id) from t;
+----------------------+------------+---------------------+
| id                   | id_text    | uint64(id)          |
+----------------------+------------+---------------------+
|  9223372036854775807 | 2 ^ 63 - 1 | 9223372036854775807 |
| -9223372036854775808 | 2 ^ 63     | 9223372036854775808 |
+----------------------+------------+---------------------+
2 rows in set (0.00 sec)
--- 2 out of 2 results in 0ms ---
```

दस्तावेज़ों को 2^63 से कम आईडी के साथ प्रश्न पूछने पर, आप असाइन किए गए मान का सीधे उपयोग कर सकते हैं:
```sql
mysql> select * from t where id = 9223372036854775807;
+---------------------+------------+
| id                  | id_text    |
+---------------------+------------+
| 9223372036854775807 | 2 ^ 63 - 1 |
+---------------------+------------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

हालाँकि, 2^63 से शुरू होने वाली आईडी के लिए, आपको साइन किए गए मान का उपयोग करना होगा:
```sql
mysql> select * from t where id = -9223372036854775808;
+----------------------+---------+
| id                   | id_text |
+----------------------+---------+
| -9223372036854775808 | 2 ^ 63  |
+----------------------+---------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

यदि आप इसके बजाय असाइन किए गए मान का उपयोग करते हैं, तो आपको त्रुटि मिलेगी:
```sql
mysql> select * from t where id = 9223372036854775808;
ERROR 1064 (42000): number 9223372036854775808 is out of range [-9223372036854775808..9223372036854775807]
```

ऐसे मान जो 64 बिट में नहीं समाते हैं, समान त्रुटि उत्पन्न करेंगे:

```sql
mysql> select * from t where id = -9223372036854775809;
ERROR 1064 (42000): number -9223372036854775809 is out of range [-9223372036854775808..9223372036854775807]
```

## वर्ण डेटा प्रकार

सामान्य व्याकरण:
```
string|text [stored|attribute] [indexed]
```

**गुण:**

1. `indexed` - पूर्ण-टेक्स्ट इंडेक्स किया गया (पूर्ण-टेक्स्ट प्रश्नों में उपयोग किया जा सकता है)
2. `stored` - एक डॉकस्टोर में संग्रहित (डिस्क पर संग्रहित, RAM में नहीं, सुस्त पढ़ें)
3. `attribute` - इसे एक स्ट्रिंग विशेषता बनाता है (इससे क्रमबद्ध/समूहित किया जा सकता है)

कम से कम एक गुण निर्दिष्ट करने से सभी डिफ़ॉल्ट गुणों को ओवरराइड करता है (नीचे देखें), यानी, यदि आप किसी विशेष संयोजन के गुणों का उपयोग करने का निर्णय लेते हैं, तो आपको उन सभी गुणों को सूचीबद्ध करना होगा जिन्हें आप चाहते हैं।

**कोई गुण निर्दिष्ट नहीं किया गया:**

`string` और `text` उपनाम हैं, लेकिन यदि आप कोई गुण निर्दिष्ट नहीं करते हैं, तो वे डिफ़ॉल्ट रूप से विभिन्न अर्थ रखते हैं:

* बस `string` डिफ़ॉल्ट रूप से `attribute` के रूप में अर्थ रखता है (विवरण देखें [नीचे](../Creating_a_table/Data_types.md#Text))।
* बस `text` डिफ़ॉल्ट रूप से `stored` + `indexed` के रूप में अर्थ रखता है (विवरण देखें [नीचे](../Creating_a_table/Data_types.md#String))।

### पाठ

<!-- टेक्स्ट के साथ काम करने का उदाहरण -->

पाठ (बस `text` या `text/string indexed`) डेटा प्रकार तालिका के पूर्ण-टेक्स्ट भाग का निर्माण करता है। टेक्स्ट फ़ील्ड इंडेक्स किए जाते हैं और कीवर्ड के लिए खोजे जा सकते हैं।

टेक्स्ट एक विश्लेषक पाइपलाइन के माध्यम से पारित होता है जो पाठ को शब्दों में परिवर्तित करता है, morphology परिवर्तनों को लागू करता है, आदि। अंततः, उस पाठ से एक पूर्ण-टेक्स्ट तालिका (एक विशेष डेटा संरचना जो कीवर्ड के लिए त्वरित खोजों की अनुमति देती है) बनाई जाती है।

पूर्ण-टेक्स्ट फ़ील्ड केवल `MATCH()` खंड में उपयोग किए जा सकते हैं और इन्हें क्रमबद्ध करने या संगAggregation के लिए उपयोग नहीं किया जा सकता है। शब्दों को एक उल्टे सूचकांक में संग्रहीत किया जाता है साथ ही उन फ़ील्ड के संदर्भ के साथ जिनसे वे संबंधित हैं और फ़ील्ड में स्थितियां। यह प्रत्येक फ़ील्ड के अंदर एक शब्द को खोजने और निकटता जैसी उन्नत संचालकों का उपयोग करने की अनुमति देता है। डिफ़ॉल्ट रूप से, फ़ील्ड के मूल पाठ को दोनों इंडेक्स किया जाता है और दस्तावेज़ संग्रहण में संग्रहित किया जाता है। इसका अर्थ है कि मूल पाठ को प्रश्न परिणामों के साथ वापस किया जा सकता है और [खोज परिणाम प्रकाशन](../Searching/Highlighting.md) में उपयोग किया जा सकता है।

<!-- परिचय -->
##### SQL:
<!-- अनुरोध SQL -->

```sql
CREATE TABLE products(title text);
```
<!-- परिचय -->
##### JSON:

<!-- अनुरोध JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text)"
```

<!-- परिचय -->
##### PHP:

<!-- अनुरोध PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text']
]);
```

<!-- परिचय -->
##### Python:

<!-- अनुरोध Python -->

```python
utilsApi.sql('CREATE TABLE products(title text)')
```
<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text)');
```
<!-- intro -->
##### जावा:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text)");
```

<!-- intro -->
##### कॉन्फ़िग:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	# जब फ़ील्ड को कॉन्फ़िगर करते समय, वे डिफ़ॉल्ट रूप से अनुक्रमित (और संग्रहीत नहीं) होते हैं
	rt_field = title

	# फ़ील्ड को संग्रहीत करने के लिए यह विकल्प निर्दिष्ट करना आवश्यक है
	stored_fields = title
}
```

<!-- end -->

<!-- indexed केवल के साथ काम करने का उदाहरण  -->

यह व्यवहार स्पष्ट रूप से यह निर्दिष्ट करके अधिलेखित किया जा सकता है कि पाठ केवल अनुक्रमित है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text indexed);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text indexed)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text','options'=>['indexed']]
]);
```
<!-- intro -->
##### पायथन:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text indexed)')
```
<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text indexed)');
```

<!-- intro -->
##### जावा:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### कॉन्फ़िग:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	# जब फ़ील्ड को कॉन्फ़िगर करते समय, वे डिफ़ॉल्ट रूप से अनुक्रमित (और संग्रहीत नहीं) होते हैं
	rt_field = title
}
```

<!-- end -->

<!-- फ़ील्ड नामकरण का उदाहरण  -->

फ़ील्ड नामित हैं, और आप अपनी खोजों को एकल फ़ील्ड (जैसे "title" के माध्यम से खोजें) या फ़ील्ड के उपसमुच्चय (जैसे "title" और "abstract" केवल) तक सीमित कर सकते हैं। आपके पास 256 पूर्ण-पाठ फ़ील्ड तक हो सकते हैं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products where match('@title first');
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
	"table": "products",
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
##### पायथन:

<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"match":{"title":"first"}}})
```
<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match":{"title":"first"}}});
```
<!-- intro -->
##### जावा:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text indexed)");
```

<!-- end -->

### स्ट्रिंग

<!-- स्ट्रिंग विशेषताओं के लिए उदाहरण  -->

पूर्ण-पाठ फ़ील्ड के विपरीत, स्ट्रिंग विशेषताएँ (केवल `string` या `string/text attribute`) उसी तरह संग्रहीत की जाती हैं जैसे उन्हें प्राप्त किया गया है और इन्हें पूर्ण-पाठ खोजों में उपयोग नहीं किया जा सकता। इसके बजाय, इन्हें परिणामों में लौटाया जाता है, तुलना फ़िल्टरिंग या `REGEX` के लिए `WHERE` क्लॉज में उपयोग किया जा सकता है, और क्रमबद्ध करने और संग्रहण के लिए उपयोग किया जा सकता है। सामान्यतः, बड़ी पाठों को स्ट्रिंग विशेषताओं में संग्रहीत करने की सिफारिश नहीं की जाती है, बल्कि नाम, शीर्षक, टैग, कुंजी जैसी मेटाडेटा के लिए स्ट्रिंग विशेषताओं का उपयोग करें।

यदि आप स्ट्रिंग विशेषता को भी अनुक्रमित करना चाहते हैं, तो आप दोनों को `string attribute indexed` के रूप में निर्दिष्ट कर सकते हैं। यह पूर्ण-पाठ खोज की अनुमति देगा और विशेषता के रूप में काम करेगा।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, keys string);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, keys string)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'keys'=>['type'=>'string']
]);
```
<!-- intro -->
##### पायथन:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, keys string)')
```
<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, keys string)');
```
<!-- intro -->
##### जावा:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, keys string)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, keys string)");
```

<!-- intro -->
##### कॉन्फ़िग:

<!-- request config -->

```ini
table products
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
<summary>और अधिक</summary>

<!-- उदाहरण स्ट्रिंग फ़ील्ड -->
आप एक ऐसा full-text फ़ील्ड बना सकते हैं जो एक string attribute के रूप में भी संग्रहीत होता है। यह तरीका एक full-text फ़ील्ड और एक string attribute बनाता है जिनका नाम समान होता है। ध्यान दें कि आप एक साथ डेटा को string attribute और document storage में संग्रहीत करने के लिए `stored` प्रॉपर्टी नहीं जोड़ सकते।

<!-- intro -->
##### SQL:
<!-- request SQL -->
`string attribute indexed` का मतलब है कि हम एक ऐसे string डेटा प्रकार के साथ काम कर रहे हैं जो attribute के रूप में संग्रहीत है और full-text फ़ील्ड के रूप में indexed है।

```sql
CREATE TABLE products ( title string attribute indexed );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products ( title string attribute indexed )"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'string','options'=>['indexed','attribute']]
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products ( title string attribute indexed )')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products ( title string attribute indexed )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products ( title string attribute indexed )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products ( title string attribute indexed )");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	rt_attr_string = title
}
```

<!-- end -->

</details>

### Manticore में बाइनरी डेटा को संग्रहीत करना

<!-- example binary -->

Manticore के पास बाइनरी डेटा के लिए समर्पित फ़ील्ड प्रकार नहीं है, लेकिन आप इसे base64 एन्कोडिंग का उपयोग करके सुरक्षित रूप से संग्रहीत कर सकते हैं और `text stored` या `string stored` फ़ील्ड प्रकारों का उपयोग कर सकते हैं (जो समानार्थी हैं)। यदि आप बाइनरी डेटा को एन्कोड नहीं करते हैं, तो इसके कुछ हिस्से खो सकते हैं — उदाहरण के लिए, Manticore किसी string के अंत को ट्रिम कर देता है यदि वह null-byte का सामना करता है।

यहाँ एक उदाहरण है जहाँ हम `ls` कमांड को base64 का उपयोग करके एन्कोड करते हैं, इसे Manticore में संग्रहीत करते हैं, और फिर MD5 चेकसम अपरिवर्तित रहता है यह सत्यापित करने के लिए इसे डिकोड करते हैं:

<!-- request Example -->
```bash
# md5sum /bin/ls
43d1b8a7ccda411118e2caba685f4329  /bin/ls
# encoded_data=`base64 -i /bin/ls `
# mysql -P9306 -h0 -e "drop table if exists test; create table test(data text stored); insert into test(data) values('$encoded_data')"
# mysql -P9306 -h0 -NB -e "select data from test" | base64 -d > /tmp/ls | md5sum
43d1b8a7ccda411118e2caba685f4329  -
```
<!-- end -->

## Integer

<!-- example for integers  -->

Integer प्रकार 32 बिट **unsigned** integer मान संग्रहीत करने की अनुमति देता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price int);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, price int)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price int)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price int)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price int)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price int)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

पूर्णांकों को 32 बिट से छोटे आकार में संग्रहीत किया जा सकता है यदि आप bit count निर्दिष्ट करते हैं। उदाहरण के लिए, यदि हम एक ऐसा संख्यात्मक मान संग्रहीत करना चाहते हैं जिसके 8 से बड़ा न होने की संभावना हो, तो प्रकार को `bit(3)` के रूप में परिभाषित किया जा सकता है। बिटकाउंट पूर्णांक पूर्ण आकार वाले पूर्णांकों की तुलना में धीमे प्रदर्शन करते हैं, लेकिन इनमें कम RAM की आवश्यकता होती है। इन्हें 32-बिट के टुकड़ों में संग्रहीत किया जाता है, इसलिए स्थान बचाने के लिए, इन्हें attribute परिभाषाओं के अंत में समूहित करना चाहिए (अन्यथा 2 पूर्ण आकार के पूर्णांकों के बीच में एक बिटकाउंट पूर्णांक भी 32 बिट का ही स्थान लेगा)।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, flags bit(3), tags bit(2) );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, flags bit(3), tags bit(2))"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, flags bit(3), tags bit(2) ')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, flags bit(3), tags bit(2) ');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, flags bit(3), tags bit(2)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, flags bit(3), tags bit(2)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

Big integers (bigint) are 64-bit wide **signed** integers.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price bigint );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, price bigint)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price bigint )')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price bigint )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price bigint )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price bigint )");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_bigint = type
}
```

<!-- end -->

## Boolean

<!-- example for boolean  -->

Declares a boolean attribute. It's equivalent to an integer attribute with bit count of 1.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, sold bool );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, sold bool)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'sold'=>['type'=>'bool']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, sold bool )')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, sold bool )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, sold bool )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, sold bool )");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_bool = sold
}
```

<!-- end -->

## Timestamps

<!-- example for timestamps  -->

The timestamp type represents Unix timestamps, which are stored as 32-bit integers. Unlike basic integers, the timestamp type allows the use of [time and date](../Functions/Date_and_time_functions.md) functions. Conversion from string values follows these rules:

- Numbers without delimiters, at least 10 characters long, are converted to timestamps as is.
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`
- `%Y-%m-%d`
- `%Y-%m`
- `%Y`

The meanings of these conversion specifiers are detailed in the [strptime manual](https://man7.org/linux/man-pages/man3/strptime.3.html), except for `%E*S`, which stands for milliseconds.

Note that auto-conversion of timestamps is not supported in plain tables.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, date timestamp);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, date timestamp)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, date timestamp)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, date timestamp)');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, date timestamp)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, date timestamp)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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
वास्तविक संख्याएँ 32-बिट IEEE 754 एकल सटीकता फ्लोट्स के रूप में संग्रहीत की जाती हैं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, coeff float);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, coeff float)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, coeff float)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, coeff float)');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, coeff float)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, coeff float)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

पूर्णांक प्रकारों के विपरीत, दो तैरते-बिंदु संख्याओं की बराबरी की तुलना न करने की सिफारिश की जाती है क्योंकि संभावित गोलाई की त्रुटियाँ हो सकती हैं। एक अधिक विश्वसनीय दृष्टिकोण यह है कि एक निकट-समान तुलना का उपयोग किया जाए, जिसमें निरपेक्ष त्रुटि सीमा की जाँच की जाए।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select abs(a-b)<=0.00001 from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->
```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"eps":"abs(a-b)"}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"eps":"abs(a-b)"}});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("ebs","abs(a-b)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object>{
    new Dictionary<string, string> { {"ebs", "abs(a-b)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- end -->

<!-- example for float mul -->

एक और विकल्प, जिसे `IN(attr,val1,val2,val3)` पर लागू करने के लिए भी उपयोग किया जा सकता है, फ्लोट्स की तुलना को पूर्णांकों के रूप में करना है, एक गुणक कारक चुनकर और संचालन में फ्लोट्स को पूर्णांकों में बदलकर। निम्नलिखित उदाहरण दिखाता है कि `IN(attr,2.0,2.5,3.5)` को पूर्णांक मूल्यों के साथ कैसे काम करने के लिए संशोधित किया जा सकता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select in(ceil(attr*100),200,250,350) from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->
```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("inc","in(ceil(attr*100),200,250,350)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"ebs", "in(ceil(attr*100),200,250,350)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- end -->

<!-- example float_accuracy -->
Manticore में फ्लोट मान सटीकता के साथ प्रदर्शित होते हैं ताकि वे संग्रहीत मान को ठीक से दर्शाएँ। इस दृष्टिकोण को सटीकता हानि को रोकने के लिए पेश किया गया था, विशेष रूप से उन मामलों के लिए जैसे भौगोलिक समन्वय, जहाँ 6 दशमलव स्थानों में गोलाई से त्रुटियाँ उत्पन्न होती थीं।
अब, Manticore पहले 6 अंकों की संख्या आउटपुट करता है, फिर इसे पार्स करता है और मूल मूल्य की तुलना करता है। यदि वे मेल नहीं खाते हैं, तो मेल खाने तक अतिरिक्त अंक जोड़े जाते हैं।

उदाहरण के लिए, यदि एक फ्लोट मान `19.45` के रूप में डाला गया था, तो Manticore इसे `19.450001` के रूप में प्रदर्शित करेगा ताकि संग्रहीत मूल्य का सही प्रतिनिधित्व हो सके।

<!-- request Example -->
```sql
insert into t(id, f) values(1, 19.45)
--------------

Query OK, 1 row affected (0.02 sec)

--------------
select * from t
--------------

+------+-----------+
| id   | f         |
+------+-----------+
|    1 | 19.450001 |
+------+-----------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

<!-- end -->


## JSON

<!-- example for creating json -->

यह डेटा प्रकार JSON ऑब्जेक्ट्स के संग्रह की अनुमति देता है, जो स्कीमा-लेस डेटा को संभालने के लिए विशेष रूप से उपयोगी है। JSON मानों को परिभाषित करते समय, सुनिश्चित करें कि ऑब्जेक्ट्स के लिए खोलने और बंद करने वाली कर्ली ब्रेसेस `{` और `}` शामिल हैं, या एरे के लिए स्क्वायर ब्रेकेट `[` और `]` शामिल हैं। जबकि JSON कॉलम सम्मिलित भंडारण द्वारा समर्थित नहीं है, इसे पारंपरिक रो-वार भंडारण में संग्रहीत किया जा सकता है। यह ध्यान देने योग्य है कि दोनों भंडारण प्रकारों को एक ही तालिका में संयोजित किया जा सकता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, data json);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, data json)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, data json)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, data json)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql'CREATE TABLE products(title text, data json)');
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql'CREATE TABLE products(title text, data json)');
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

JSON गुणों का उपयोग अधिकांश ऑपरेशन्स में किया जा सकता है। विशेष फ़ंक्शन भी हैं जैसे [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29), [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29), [GREATEST()](../Functions/Mathematical_functions.md#GREATEST%28%29), [LEAST()](../Functions/Mathematical_functions.md#LEAST%28%29) और [INDEXOF()](../Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29) जो गुणों के एरे के माध्यम से यात्रा करने की अनुमति देते हैं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select indexof(x>2 for x in data.intarray) from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("idx","indexof(x>2 for x in data.intarray)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"idx", "indexof(x>2 for x in data.intarray)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- end -->

<!-- example for REGEX() json -->

टेक्स्ट गुणों को स्ट्रिंग के रूप में माना जाता है, इसलिए इन्हें पूर्ण-टेक्स्ट मेल अभिव्यक्तियों में उपयोग करना संभव नहीं है। हालाँकि, स्ट्रिंग फ़ंक्शंस जैसे [REGEX()](../Functions/String_functions.md#REGEX%28%29) का उपयोग किया जा सकता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select regex(data.name, 'est') as c from products where c>0
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("range", new HashMap<String,Object>(){{
    put("c", new HashMap<String,Object>(){{
        put("gt",0);
    }});
}});
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("idx","indexof(x>2 for x in data.intarray)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
var rangeFilter = new RangeFilter("c");
rangeFilter.Gt = 0;
searchRequest.AttrFilter = rangeFilter;
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"idx", "indexof(x>2 for x in data.intarray)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- end -->

<!-- example for DOUBLE() -->

JSON गुणों के मामले में, उचित कार्यक्षमता के लिए डेटा प्रकार को लागू करना आवश्यक हो सकता है। उदाहरण के लिए, फ्लोट मानों के साथ काम करते समय, [DOUBLE()](../Functions/Type_casting_functions.md#DOUBLE%28%29) को उचित सॉर्टिंग के लिए उपयोग किया जाना चाहिए।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products order by double(data.myfloat) desc
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("double(data.myfloat)",new HashMap<String,String>(){{ put("order","desc");}});}});
}});
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Sort = new List<Object> {
    new SortOrder("double(data.myfloat)", SortOrder.OrderEnum.Desc)
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- end -->

## Float vector

<!-- example for creating float_vector -->
Float vector गुणांक भिन्न लंबाई की फ्लोट सूचियों को संग्रहीत करने की अनुमति देते हैं, जो मुख्य रूप से मशीन लर्निंग अनुप्रयोगों और समानता खोजों के लिए उपयोग होते हैं। यह प्रकार कई महत्वपूर्ण तरीकों से मल्टी-वैल्यूड गुणांक (MVAs) से भिन्न होता है:
- मानों के सटीक क्रम को बनाए रखता है (MVAs के विपरीत जो पुनर्व्यवस्थित कर सकते हैं)
- डुप्लिकेट मानों को बनाए रखता है (MVAs के विपरीत जो डुप्लिकेट करते हैं)
- सम्मिलन के दौरान अतिरिक्त प्रसंस्करण की आवश्यकता नहीं है (MVAs के विपरीत जो सॉर्ट और डुप्लिकेट करते हैं)

### उपयोग और सीमाएँ
- वर्तमान में केवल वास्तविक समय की तालिकाओं में समर्थित
- केवल KNN (k-नजदीकी पड़ोसी) खोजों में उपयोग किया जा सकता है
- साधारण तालिकाओं या अन्य फ़ंक्शनों/व्यक्तियों में समर्थित नहीं
- KNN सेटिंग्स के साथ उपयोग करने पर, आप `float_vector` मानों को `UPDATE` नहीं कर सकते। इसके बजाय `REPLACE` का उपयोग करें
- KNN सेटिंग्स के बिना उपयोग करने पर, आप `float_vector` मानों को `UPDATE` कर सकते हैं
- फ्लोट वेक्टरों का उपयोग नियमित फ़िल्टर या सॉर्टिंग में नहीं किया जा सकता
- `float_vector` मानों द्वारा फ़िल्टर करने का एकमात्र तरीका वेक्टर खोज संचालन (KNN) के माध्यम से है

### सामान्य उपयोग मामलों
- समानता खोज के लिए छवि एम्बेडिंग
- अर्थशास्त्र खोज के लिए पाठ एम्बेडिंग
- मशीन लर्निंग के लिए विशेषता वेक्टर
- सिफारिश प्रणाली वेक्टर

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, image_vector float_vector);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, image_vector float_vector)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'image_vector'=>['type'=>'float_vector']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, image_vector float_vector)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, image_vector float_vector)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, image_vector float_vector)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, image_vector float_vector)");
```

<!-- intro -->

##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_float_vector = image_vector
}
```

<!-- end -->

For information about using float vectors in searches, see [KNN search](../Searching/KNN.md).

## Multi-value integer (MVA)

<!-- example for creating MVA32 -->

Multi-value attributes allow storing variable-length lists of 32-bit unsigned integers. This can be useful for storing one-to-many numeric values, such as tags, product categories, and properties.

<!-- intro -->

##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, product_codes multi);
```

<!-- intro -->

##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, product_codes multi)"
```

<!-- intro -->

##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, product_codes multi)')
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, product_codes multi)');
```

<!-- intro -->

##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, product_codes multi)");
```

<!-- intro -->

##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, product_codes multi)");
```

<!-- intro -->

##### config:

<!-- request config -->

```ini
table products
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
It supports filtering and aggregation, but not sorting. Filtering can be done using a condition that requires at least one element to pass (using [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)) or all elements ([ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29)) to pass.

<!-- intro -->

##### SQL:
<!-- request SQL -->

```sql
select * from products where any(product_codes)=3
```

<!-- intro -->

##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
  "query":
  {
    "match_all": {},
    "equals" : { "any(product_codes)": 3 }
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
searchApi.search({"table":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})
```

<!-- intro -->

##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("equals",new HashMap<String,Integer>(){{
     put("any(product_codes)",3);
}});
searchRequest.setQuery(query);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->

##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.AttrFilter = new EqualsFilter("any(product_codes)", 3);
var searchResponse = searchApi.Search(searchRequest);
```

<!-- end -->

<!-- example for least/greatest MVA -->

Information like [least](../Functions/Mathematical_functions.md#LEAST%28%29) or [greatest](../Functions/Mathematical_functions.md#GREATEST%28%29) element and length of the list can be extracted. An example shows ordering by the least element of a multi-value attribute.

<!-- intro -->

##### SQL:
<!-- request SQL -->

```sql
select least(product_codes) l from products order by l asc
```

<!-- intro -->

##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}})
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}});
```

<!-- intro -->

##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("product_codes",new HashMap<String,String>(){{ put("order","asc");put("mode","min");}});}});
}});
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Sort = new List<Object> {
    new SortMVA("product_codes", SortOrder.OrderEnum.Asc, SortMVA.ModeEnum.Min)
};
searchResponse = searchApi.Search(searchRequest);
```

<!-- end -->

<!-- example for grouping by MVA -->
जब एक बहु-मूल्य विशेषता द्वारा समूहबद्ध किया जाता है, तो एक दस्तावेज़ उस दस्तावेज़ के साथ जुड़े विभिन्न मूल्यों की संख्या के रूप में कई समूहों में योगदान देगा। उदाहरण के लिए, यदि एक संग्रह में ठीक एक दस्तावेज़ है जिसमें 'product_codes' बहु-मूल्य विशेषता के साथ मान 5, 7 और 11 हैं, तो 'product_codes' पर समूह बनाना 3 समूह उत्पन्न करेगा जिनकी `COUNT(*)` मान 1 के बराबर होगी और `GROUPBY()` की कुंजी मान 5, 7 और 11, क्रमशः। साथ ही, ध्यान दें कि बहु-मूल्य विशेषताओं द्वारा समूहबद्ध करने से परिणाम सेट में नकल दस्तावेज़ हो सकते हैं क्योंकि प्रत्येक दस्तावेज़ कई समूहों में भाग ले सकता है।

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
बहु-मूल्य विशेषताओं के मान के रूप में डाले गए संख्याओं के क्रम को संरक्षित नहीं किया जाता है। मान अंतर्निहित रूप से एक क्रमबद्ध सेट के रूप में संग्रहीत किए जाते हैं।

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
##### JSON:

<!-- request JSON -->

```JSON
POST /insert
{
	"table":"products",
	"id":1,
	"doc":
	{
		"title":"first",
		"product_codes":[4,2,1,3]
	}
}

POST /search
{
  "table": "products",
  "query": { "match_all": {} }
}
```

<!-- response JSON -->

```JSON
{
   "table":"products",
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
            "_id": 1,
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
indexApi.insert({"table":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}})
searchApi.search({"table":"products","query":{"match_all":{}}})
```
<!-- response Python -->

```python
{'created': True,
 'found': None,
 'id': 1,
 'table': 'products',
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
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
await indexApi.insert({"table":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}});
res = await searchApi.search({"table":"products","query":{"match_all":{}}});
```
<!-- response javascript -->

```javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1,"_source":{"product_codes":[1,2,3,4],"title":"first"}}]}}
```
<!-- intro -->
##### java:

<!-- request java -->

```java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","first");
    put("product_codes",new int[] {4,2,1,3});
}};
newdoc.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("products");
searchRequest.setQuery(query);
SearchResponse searchResponse = searchApi.search(searchRequest);
System.out.println(searchResponse.toString() );
```
<!-- response java -->

```java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=1, _score=1, _source={product_codes=[1, 2, 3, 4], title=first}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "first");
doc.Add("product_codes", new List<Object> {4,2,1,3});
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
object query =  new { match_all=null };
var searchRequest = new SearchRequest("products", query);
var searchResponse = searchApi.Search(searchRequest);
Console.WriteLine(searchResponse.ToString())
```
<!-- response C# -->

```clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=1, _score=1, _source={product_codes=[1, 2, 3, 4], title=first}}]
        aggregations: null
    }
    profile: null
}

```

<!-- end -->


## Multi-value big integer

<!-- example for creating MVA64 -->

A data type that allows storing variable-length lists of 64-bit signed integers. It has the same functionality as multi-value integer.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, values multi64);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, values multi64)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, values multi64))')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, values multi64))');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, values multi64))");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, values multi64))");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_multi_64 = values
}
```

<!-- end -->

## Columnar attribute properties

When you use the columnar storage you can specify the following properties for the attributes.

<!-- example fast_fetch -->
### fast_fetch

By default, Manticore Columnar storage stores all attributes in a columnar fashion, as well as in a special docstore row by row. This enables fast execution of queries like `SELECT * FROM ...`, especially when fetching a large number of records at once. However, if you are sure that you do not need it or wish to save disk space, you can disable it by specifying `fast_fetch='0'` when creating a table or (if you are defining a table in a config) by using `columnar_no_fast_fetch` as shown in the following example.

<!-- request RT mode -->
```sql
create table t(a int, b int fast_fetch='0') engine='columnar'; desc t;
```

<!-- response RT mode -->

```sql
+-------+--------+---------------------+
| Field | Type   | Properties          |
+-------+--------+---------------------+
| id    | bigint | columnar fast_fetch |
| a     | uint   | columnar fast_fetch |
| b     | uint   | columnar            |
+-------+--------+---------------------+
3 rows in set (0.00 sec)
```

<!-- request Plain mode -->

```ini
source min {
    type = mysql
    sql_host = localhost
    sql_user = test
# डेटा प्रकार

## फुल-टेक्स्ट फील्ड्स और विशेषताएं

मैन्टिकोर के डेटा प्रकार दो श्रेणियों में विभाजित किए जा सकते हैं: फुल-टेक्स्ट फील्ड्स और विशेषताएं।

### फील्ड नाम सिंटैक्स

मैन्टिकोर में फील्ड नामों को इन नियमों का पालन करना चाहिए:

* अक्षर (a-z, A-Z), संख्या (0-9), और हाइफन (-) शामिल कर सकते हैं
* एक अक्षर से शुरू होना चाहिए
* संख्याएँ केवल अक्षरों के बाद ही आ सकती हैं
* अंडरस्कोर (`_`) ही एकमात्र अनुमत विशेष वर्ण है
* फील्ड नाम केस-असंवेदनशील होते हैं

उदाहरण के लिए:
* वैध फील्ड नाम: `title`, `product_id`, `user_name_2`
* अवैध फील्ड नाम: `2title`, `-price`, `user@name`

### फुल-टेक्स्ट फील्ड्स

फुल-टेक्स्ट फील्ड्स:
* प्राकृतिक भाषा प्रोसेसिंग एल्गोरिदम के साथ इंडेक्स किया जा सकता है, इसलिए कीवर्ड्स के लिए खोजा जा सकता है
* सॉर्टिंग या ग्रुपिंग के लिए प्रयुक्त नहीं हो सकते
* मूल दस्तावेज़ की सामग्री को पुनः प्राप्त किया जा सकता है
* मूल दस्तावेज़ की सामग्री को हाइलाइटिंग के लिए उपयोग किया जा सकता है

फुल-टेक्स्ट फील्ड्स को `text` डेटा प्रकार द्वारा दर्शाया जाता है। सभी अन्य डेटा प्रकार "विशेषताओं" के रूप में जाने जाते हैं।

### विशेषताएं

विशेषताएं गैर-फुल-टेक्स्ट मान होते हैं जो प्रत्येक दस्तावेज़ के साथ जुड़े होते हैं, जो खोज के दौरान गैर-फुल-टेक्स्ट फिल्टरिंग, सॉर्टिंग और ग्रुपिंग करने के लिए उपयोग किए जा सकते हैं।

अक्सर यह चाहा जाता है कि फुल-टेक्स्ट खोज परिणामों को केवल मैचिंग दस्तावेज़ आईडी और उसकी रैंक के आधार पर नहीं बल्कि कई अन्य प्रति-दस्तावेज़ मानों के आधार पर प्रोसेस किया जाए। उदाहरण के लिए, किसी को समाचार खोज परिणामों को तारीख और फिर सापेक्षता द्वारा सॉर्ट करने की आवश्यकता हो सकती है, या निर्दिष्ट मूल्य सीमा के भीतर उत्पादों के माध्यम से खोज करना, या केवल चयनित उपयोगकर्ताओं द्वारा किए गए पोस्ट की ब्लॉग खोज को सीमा में करना, या महीने के अनुसार परिणामों को समूहबद्ध करना। इसे कुशलतापूर्वक करने के लिए, मैन्टिकोर प्रत्येक दस्तावेज़ के लिए फुल-टेक्स्ट फील्ड्स के अलावा अतिरिक्त विशेषताएं जोड़ने के लिए सक्षम करता है। ये विशेषताएं फुल-टेक्स्ट मैचों को फ़िल्टर करने, सॉर्ट करने या समूहबद्ध करने के लिए उपयोग की जा सकती हैं, या केवल विशेषताओं द्वारा खोजा जा सकता है।

विशेषताएं, फुल-टेक्स्ट फील्ड्स के विपरीत, फुल-टेक्स्ट इंडेक्स नहीं की जाती हैं। वे तालिका में संग्रहीत होती हैं, लेकिन उन्हें फुल-टेक्स्ट के रूप में खोजना संभव नहीं है।

<!-- example attributes or fields -->

विशेषताओं के लिए अच्छा उदाहरण एक फोरम पोस्ट्स तालिका होगा। मान लें कि केवल शीर्षक और सामग्री फील्ड्स को फुल-टेक्स्ट खोजने योग्य होना चाहिए - लेकिन कभी-कभी यह भी आवश्यक होता है कि खोज को किसी विशेष लेखक या उप-फोरम तक सीमित किया जाए (यानी, केवल उन पंक्तियों की खोज करें जिनमें किसी विशेष author_id या forum_id के मान होते हैं); या मैचों को post_date कॉलम द्वारा सॉर्ट करने के लिए; या मिलते-जुलते पोस्टों को post_date के महीने द्वारा समूहबद्ध करने के लिए और प्रति-समूह मैच गणना की गणना करने के लिए।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)"
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

```python
utilsApi.sql('CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)')
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)");

```

<!-- intro -->
##### C#:

<!-- request C# -->

```java
utilsApi.Sql("CREATE TABLE forum(title text, content text, author_id int, forum_id int, post_date timestamp)");

```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table forum
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

यह उदाहरण `author_id`, `forum_id` और `post_date` के द्वारा सॉर्ट किए गए फुल-टेक्स्ट क्वेरी को संचालित करते हुए दिखाता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from forum where author_id=123 and forum_id in (1,3,7) order by post_date desc
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "forum",
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
        'table' => 'forum',
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
searchApi.search({"table":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"forum","query":{"match_all":{},"bool":{"must":[{"equals":{"author_id":123}},{"in":{"forum_id":[1,3,7]}}]}},"sort":[{"post_date":"desc"}]});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
HashMap<String,Object> filters = new HashMap<String,Object>(){{
    put("must", new HashMap<String,Object>(){{
        put("equals",new HashMap<String,Integer>(){{
            put("author_id",123);
        }});
        put("in",
            new HashMap<String,Object>(){{
                put("forum_id",new int[] {1,3,7});
        }});
    }});
}};
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("bool",filters);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("post_date","desc");}});
}});
SearchResponse searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
var boolFilter = new BoolFilter();
boolFilter.Must = new List<Object> {
    new EqualsFilter("author_id", 123),
    new InFilter("forum_id", new List<Object> {1,3,7})
};
searchRequest.AttrFilter = boolFilter;
searchRequest.Sort = new List<Object> { new SortOrder("post_date", SortOrder.OrderEnum.Desc) };
var searchResponse = searchApi.Search(searchRequest);
```

<!-- end -->

### पंक्ति-वार और कॉलम वाले विशेषता भंडारण

मैन्टिकोर दो प्रकार के विशेषता भंडारण का समर्थन करता है:
* पंक्ति-वार - पारंपरिक भंडारण जो मैन्टिकोर खोज में बॉक्स से बाहर उपलब्ध है
* कॉलम वाला - [मैन्टिकोर कॉलम लाइब्रेरी](https://github.com/manticoresoftware/columnar) द्वारा प्रदान किया गया

जैसा कि उनके नाम से समझा जा सकता है, वे डेटा को अलग तरह से संग्रहीत करते हैं। पारंपरिक **पंक्ति-वार भंडारण**:
* विशेषताओं को बिना संकुचित किए संग्रहीत करता है
* एक ही दस्तावेज़ के सभी विशेषताएँ एक पंक्ति में एक-दूसरे के करीब संग्रहीत की जाती हैं
* पंक्तियाँ एक-एक करके संग्रहीत की जाती हैं
* विशेषताओं तक पहुँच आमतौर पर केवल पंक्ति आईडी को स्ट्राइड (एकल वेक्टर की लंबाई) से गुणा करके और गणना की गई मेमोरी स्थान से अनुरोधित विशेषता प्राप्त करके की जाती है। यह बहुत कम बेतरतीब पहुंच विलंबता देता है।
* विशेषताओं को स्वीकार्य प्रदर्शन प्राप्त करने के लिए मेमोरी में होना चाहिए, अन्यथा पंक्ति-वार भंडारण की प्रकृति के कारण मैन्टिकोर को बहुत सारे अनावश्यक डेटा को डिस्क से पढ़ना पड़ सकता है जो कई मामलों में उप-आदर्श होता है।

**कॉलम वाले भंडारण** के साथ:
* प्रत्येक विशेषता सभी अन्य विशेषताओं से स्वतंत्र रूप से अपने अलग "कॉलम" में संग्रहीत की जाती है
* भंडारण 65536 प्रविष्टियों के ब्लॉकों में विभाजित किया जाता है
* ब्लॉकों को संकुचित रूप में संग्रहीत किया जाता है। यह अक्सर केवल कुछ विशिष्ट मानों को संग्रहीत करने की अनुमति देता है, बजाय इसके कि सभी को पंक्ति-वार भंडारण की तरह। उच्च संकुचन अनुपात डिस्क से तेजी से पढ़ने की अनुमति देता है और मेमोरी की आवश्यकता को बहुत कम करता है
* जब डेटा अनुक्रमित किया जाता है, तो प्रत्येक ब्लॉक के लिए भंडारण योजना स्वतंत्र रूप से चयनित की जाती है। उदाहरण के लिए, यदि किसी ब्लॉक में सभी मान समान हैं, तो इसे "कॉन्ट" भंडारण मिलता है और पूरे ब्लॉक के लिए केवल एक मान संग्रहीत होता है। यदि किसी ब्लॉक में 256 से कम विशिष्ट मान हैं, तो इसे "टेबल" भंडारण मिलता है और मानों की तालिका के लिए सूचकांक संग्रहीत करता है बजाय मानों के स्वयं के
* यदि स्पष्ट है कि अनुरोधित मान ब्लॉक में मौजूद नहीं है तो एक ब्लॉक में खोज जल्दी अस्वीकार की जा सकती है।

कॉलम वाला भंडारण बड़े डेटा मात्रा को संभालने के लिए डिज़ाइन किया गया था जो RAM में फिट नहीं होता है, इसलिए सिफारिशें हैं:
* यदि आपके पास सभी विशेषताओं के लिए पर्याप्त मेमोरी है, तो आप पंक्ति-वार भंडारण से लाभान्वित होंगे
* अन्यथा, कॉलम वाला भंडारण अभी भी आपको बहुत कम मेमोरी पदचिह्न के साथ उचित प्रदर्शन दे सकता है, जो आपको अपने टेबल में बहुत अधिक दस्तावेज़ संग्रहीत करने की अनुमति देगा

### भंडारण के बीच स्विच कैसे करें

पारंपरिक पंक्ति-वार भंडारण डिफ़ॉल्ट है, इसलिए यदि आप चाहते हैं कि सब कुछ पंक्ति-वार तरीके से संग्रहीत हो, तो आपको तालिका बनाने के समय कुछ भी करने की आवश्यकता नहीं है।

कॉलम वाला भंडारण सक्षम करने के लिए, आपको आवश्यकता है:
* [CREATE TABLE](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Creating-a-real-time-table-online-via-CREATE-TABLE) में `engine='columnar'` निर्दिष्ट करें ताकि तालिका की सभी विशेषताएँ कॉलम वाली हो जाएं। फिर, यदि आप किसी विशेष विशेषता को पंक्ति-वार बनाए रखना चाहते हैं, तो जब आप इसे घोषित करते हैं तब `engine='rowwise'` जोड़ें। उदाहरण के लिए:
```sql
create table tbl(title text, type int, price float engine='rowwise') engine='columnar'
```
* एक विशेष विशेषता के लिए `CREATE TABLE` में `engine='columnar'` निर्दिष्ट करें ताकि इसे कॉलम वाली बनाया जा सके। उदाहरण के लिए:
```sql
create table tbl(title text, type int, price float engine='columnar');
```
या
```sql
create table tbl(title text, type int, price float engine='columnar') engine='rowwise';
```
* [सादा मोड](../Read_this_first.md#Real-time-mode-vs-plain-mode) में, आपको [columnar_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) में वह विशेषताएँ सूचीबद्ध करने की आवश्यकता है जिन्हें आप कॉलम वाली बनाना चाहते हैं।

नीचे मैन्टिकोर खोज द्वारा समर्थित डेटा प्रकारों की सूची दी गई है:

## दस्तावेज़ आईडी

दस्तावेज़ पहचानकर्ता एक अनिवार्य विशेषता है जो एक अद्वितीय 64-बिट unsigned integer होना चाहिए। दस्तावेज़ आईडी को तालिका बनाने के समय स्पष्ट रूप से निर्दिष्ट किया जा सकता है, लेकिन वे हमेशा सक्षम होते हैं भले ही उन्हें निर्दिष्ट न किया गया हो। दस्तावेज़ आईडी को अपडेट नहीं किया जा सकता है।

जब आप एक तालिका बनाते हैं, तो आप आईडी को स्पष्ट रूप से निर्दिष्ट कर सकते हैं, लेकिन आप जिस डेटा प्रकार का उपयोग करते हैं, उससे कोई फर्क नहीं पड़ता, यह हमेशा ऊपर वर्णित तरीके से कार्य करेगा - इसे unsigned 64-bit के रूप में संग्रहीत किया जाएगा लेकिन signed 64-bit integer के रूप में प्रदर्शित किया जाएगा।

```sql
mysql> CREATE TABLE tbl(id bigint, content text);
DESC tbl;
+---------+--------+----------------+
| Field   | Type   | Properties     |
+---------+--------+----------------+
| id      | bigint |                |
| content | text   | indexed stored |
+---------+--------+----------------+
2 rows in set (0.00 sec)
```

आप ID निर्दिष्ट न करने का भी विकल्प चुन सकते हैं, यह स्वचालित रूप से सक्षम हो जाएगा।
```sql
mysql> CREATE TABLE tbl(content text);
DESC tbl;
+---------+--------+----------------+
| Field   | Type   | Properties     |
+---------+--------+----------------+
| id      | bigint |                |
| content | text   | indexed stored |
+---------+--------+----------------+
2 rows in set (0.00 sec) 
```

दस्तावेज़ ID के साथ काम करते समय, यह जानना महत्वपूर्ण है कि वे आंतरिक रूप से बिना चिह्न के 64-बिट के पूर्णांक के रूप में संग्रहीत किए जाते हैं, लेकिन क्वेरी और परिणामों में हस्ताक्षरित 64-बिट के पूर्णांक के रूप में दिखाए जाते हैं। इसका अर्थ है:

* 2^63-1 से बड़े ID नकारात्मक संख्याओं के रूप में दिखेंगे।
* ऐसे बड़े ID को फ़िल्टर करते समय, आपको उनका हस्ताक्षरित प्रतिनिधित्व उपयोग करना होगा।
* वास्तविक बिना चिह्न के मान को देखने के लिए [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29) फ़ंक्शन का उपयोग करें।

उदाहरण के लिए, एक टेबल बनाएं और 2^63 के आसपास कुछ मान डालें:
```sql
mysql> create table t(id_text string)
Query OK, 0 rows affected (0.01 sec)

mysql> insert into t values(9223372036854775807, '2 ^ 63 - 1'),(9223372036854775808, '2 ^ 63')
Query OK, 2 rows affected (0.00 sec)
```

कुछ ID नकारात्मक संख्याओं के रूप में दिखाई देते हैं क्योंकि वे 2^63-1 से अधिक हैं। हालांकि, `UINT64(id)` का उपयोग करके उनके वास्तविक बिना चिह्न के मान पता चल सकते हैं:
```sql
mysql> select *, uint64(id) from t
+----------------------+------------+---------------------+
| id                   | id_text    | uint64(id)          |
+----------------------+------------+---------------------+
|  9223372036854775807 | 2 ^ 63 - 1 | 9223372036854775807 |
| -9223372036854775808 | 2 ^ 63     | 9223372036854775808 |
+----------------------+------------+---------------------+
2 rows in set (0.00 sec)
--- 2 out of 2 results in 0ms ---
```

2^63 से कम ID वाले दस्तावेज़ों को क्वेरी करने के लिए, आप सीधे बिना चिह्न के मान का उपयोग कर सकते हैं:
```sql
mysql> select * from t where id = 9223372036854775807
+---------------------+------------+
| id                  | id_text    |
+---------------------+------------+
| 9223372036854775807 | 2 ^ 63 - 1 |
+---------------------+------------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

हालांकि, 2^63 से शुरू होने वाले ID के लिए, आपको हस्ताक्षरित मान का उपयोग करना होगा:
```sql
mysql> select * from t where id = -9223372036854775808
+----------------------+---------+
| id                   | id_text |
+----------------------+---------+
| -9223372036854775808 | 2 ^ 63  |
+----------------------+---------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

यदि आप इसके बजाय बिना चिह्न के मान का उपयोग करते हैं, तो आपको गलत परिणाम मिल सकते हैं:
```sql
mysql> select * from t where id = 9223372036854775808
+---------------------+------------+
| id                  | id_text    |
+---------------------+------------+
| 9223372036854775807 | 2 ^ 63 - 1 |
+---------------------+------------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

## वर्ण डेटा प्रकार

सामान्य सिंटैक्स:
```
string|text [stored|attribute] [indexed]
```

**गुण:**

1. `indexed` - पूर्ण-पाठ अनुक्रमित (पूर्ण-पाठ क्वेरी में उपयोग किया जा सकता है)
2. `stored` - एक डॉकस्टोर में संग्रहीत (डिस्क पर संग्रहीत, RAM में नहीं, आलसी पठन)
3. `attribute` - इसे एक स्ट्रिंग विशेषता बनाता है (इसके द्वारा क्रमबद्ध/समूह किया जा सकता है)

कम से कम एक गुण निर्दिष्ट करने से सभी डिफ़ॉल्ट गुणों को ओवरराइड किया जाता है (देखें नीचे), यानी यदि आप गुणों के एक कस्टम संयोजन का उपयोग करने का निर्णय लेते हैं, तो आपको सभी गुणों को सूचीबद्ध करना होगा।

**कोई गुण निर्दिष्ट नहीं किया गया:**

`string` और `text` समानार्थक हैं, लेकिन यदि आप कोई गुण निर्दिष्ट नहीं करते हैं, तो वे अलग-अलग चीज़ों को दर्शाते हैं:

* केवल `string` डिफ़ॉल्ट रूप से `attribute` का अर्थ है (विवरण के लिए [नीचे](../Creating_a_table/Data_types.md#Text) देखें)।
* केवल `text` डिफ़ॉल्ट रूप से `stored` + `indexed` का अर्थ है (विवरण के लिए [नीचे](../Creating_a_table/Data_types.md#String) देखें)।

### पाठ

<!-- example working with text -->

पाठ (केवल `text` या `text/string indexed`) डेटा प्रकार टेबल के पूर्ण-पाठ भाग को बनाता है। पाठ फील्ड अनुक्रमित हैं और कीवर्ड के लिए खोजे जा सकते हैं।

पाठ को एक विश्लेषक पाइपलाइन से गुजारा जाता है जो पाठ को शब्दों में परिवर्तित करता है, रूपधान रूपांतरण लागू करता है, आदि। अंततः, उस पाठ से एक पूर्ण-पाठ टेबल (एक विशेष डेटा संरचना जो किसी कीवर्ड के लिए त्वरित खोज को सक्षम बनाती है) बनाई जाती है।

पूर्ण-पाठ फील्ड केवल `MATCH()` खंड में उपयोग की जा सकती हैं और क्रमबद्ध या समूहीकरण के लिए नहीं। शब्दों को एक उल्टा अनुक्रमणिका में संग्रहीत किया जाता है जिसमें फील्ड जिनसे वे संबंधित हैं और फील्ड में उनकी स्थितियों के संदर्भ भी होते हैं। यह प्रत्येक फील्ड के अंदर एक शब्द की खोज और निकटता जैसे उन्नत ऑपरेटरों का उपयोग करने की अनुमति देता है। डिफ़ॉल्ट रूप से, फील्ड के मूल पाठ को अनुक्रमित और दस्तावेज़ भंडारण में संग्रहीत किया जाता है। इसका अर्थ है कि मूल पाठ को क्वेरी परिणामों के साथ वापस किया जा सकता है और [खोज परिणाम हाइलाइटिंग](../Searching/Highlighting.md) में उपयोग किया जा सकता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text)"
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
utilsApi.sql('CREATE TABLE products(title text)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

This behavior can be overridden by explicitly specifying that the text is only indexed.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text indexed);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text indexed)"
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
utilsApi.sql('CREATE TABLE products(title text indexed)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text indexed)');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	# when configuring fields via config, they are indexed (and not stored) by default
	rt_field = title
}
```

<!-- end -->

<!-- example for field naming  -->

Fields are named, and you can limit your searches to a single field (e.g. search through "title" only) or a subset of fields (e.g. "title" and "abstract" only). You can have up to 256 full-text fields.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products where match('@title first');
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
	"table": "products",
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
searchApi.search({"table":"products","query":{"match":{"title":"first"}}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match":{"title":"first"}}});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text indexed)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text indexed)");
```

<!-- end -->

### String

<!-- example for string attributes  -->

Unlike full-text fields, string attributes (just `string` or `string/text attribute`) are stored as they are received and cannot be used in full-text searches. Instead, they are returned in results, can be used in the `WHERE` clause for comparison filtering or `REGEX`, and can be used for sorting and aggregation. In general, it's not recommended to store large texts in string attributes, but use string attributes for metadata like names, titles, tags, keys.

If you want to also index the string attribute, you can specify both as `string attribute indexed`. It will allow full-text searching and works as an attribute.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, keys string);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, keys string)"
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
utilsApi.sql('CREATE TABLE products(title text, keys string)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, keys string)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, keys string)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, keys string)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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
आप एक पूर्ण-पाठ फ़ील्ड बना सकते हैं जो एक स्ट्रिंग एट्रिब्यूट के रूप में भी संग्रहीत होता है। यह दृष्टिकोण एक पूर्ण-पाठ फ़ील्ड और एक स्ट्रिंग एट्रिब्यूट बनाता है जिनके पास एक ही नाम होता है। ध्यान दें कि आप डेटा को एक स्ट्रिंग एट्रिब्यूट के रूप में और दस्तावेज़ संग्रह में एक ही समय में संग्रहेत करने के लिए `stored` प्रॉपर्टी नहीं जोड़ सकते।

<!-- intro -->
##### SQL:
<!-- request SQL -->
`string attribute indexed` का मतलब है कि हम एक स्ट्रिंग डेटा प्रकार के साथ काम कर रहे हैं जो एक एट्रिब्यूट के रूप में संग्रहीत होता है और एक पूर्ण-पाठ फ़ील्ड के रूप में अनुक्रमित होता है।

```sql
CREATE TABLE products ( title string attribute indexed );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products ( title string attribute indexed )"
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
utilsApi.sql('CREATE TABLE products ( title string attribute indexed )')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products ( title string attribute indexed )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products ( title string attribute indexed )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products ( title string attribute indexed )");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	rt_attr_string = title
}
```

<!-- end -->

</details>

### Manticore में बाइनरी डेटा संग्रहित करना

<!-- example binary -->

Manticore के पास बाइनरी डेटा के लिए कोई समर्पित फ़ील्ड प्रकार नहीं है, लेकिन आप इसे सुरक्षित रूप से बेस64 एन्कोडिंग और `text stored` या `string stored` फ़ील्ड प्रकारों का उपयोग करके संग्रहीत कर सकते हैं (जो समानार्थी हैं)। यदि आप बाइनरी डेटा को एन्कोड नहीं करते हैं, तो इसके कुछ भाग खो सकते हैं - उदाहरण के लिए, Manticore एक स्ट्रिंग के अंत को ट्रिम करता है यदि वह एक नल-बाइट का सामना करता है।

यहाँ एक उदाहरण है जहां हम `ls` कमांड को बेस64 का उपयोग करके एन्कोड करते हैं, इसे Manticore में संग्रहीत करते हैं, और फिर एमडी5 चेकसम की पुष्टि के लिए इसे डिकोड करते हैं कि यह अपरिवर्तित है:

<!-- request Example -->
```bash
# md5sum /bin/ls
43d1b8a7ccda411118e2caba685f4329  /bin/ls
# encoded_data=`base64 -i /bin/ls `
# mysql -P9306 -h0 -e "drop table if exists test; create table test(data text stored); insert into test(data) values('$encoded_data')"
# mysql -P9306 -h0 -NB -e "select data from test" | base64 -d > /tmp/ls | md5sum
43d1b8a7ccda411118e2caba685f4329  -
```
<!-- end -->

## पूर्णांक

<!-- example for integers  -->

पूर्णांक प्रकार 32 बिट **unsigned** पूर्णांक मान संग्रहीत करने की अनुमति देता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price int);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, price int)"
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
utilsApi.sql('CREATE TABLE products(title text, price int)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price int)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price int)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price int)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

पूर्णांक को बिट गणना निर्दिष्ट करके 32-बिट से छोटा आकार में संग्रहीत किया जा सकता है। उदाहरण के लिए, यदि हम एक संख्यात्मक मान संग्रहीत करना चाहते हैं जो हम जानते हैं कि 8 से बड़ा नहीं होने वाला है, तो इसे `bit(3)` के रूप में परिभाषित किया जा सकता है। बिटगणना पूर्णांक पूर्ण आकार वाले लोगों की तुलना में धीमें प्रदर्शन करते हैं, लेकिन उन्हें कम RAM की आवश्यकता होती है। वे 32-बिट टुकड़ों में संग्रहीत होते हैं, इसलिए स्थान बचाने के लिए, उन्हें एट्रिब्यूट परिभाषाओं के अंत में समूहित किया जाना चाहिए (अन्यथा 2 पूर्ण आकार के पूर्णांकों के बीच एक बिटगणना पूर्णांक भी 32 बिट्स का स्थान लेगा)।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, flags bit(3), tags bit(2) );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, flags bit(3), tags bit(2))"
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
utilsApi.sql('CREATE TABLE products(title text, flags bit(3), tags bit(2) ')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, flags bit(3), tags bit(2) ');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, flags bit(3), tags bit(2)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, flags bit(3), tags bit(2)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

Big integers (bigint) are 64-bit wide **signed** integers.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price bigint );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, price bigint)"
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
utilsApi.sql('CREATE TABLE products(title text, price bigint )')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price bigint )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price bigint )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price bigint )");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_bigint = type
}
```

<!-- end -->

## Boolean

<!-- example for boolean  -->

Declares a boolean attribute. It's equivalent to an integer attribute with bit count of 1.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, sold bool );
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, sold bool)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'sold'=>['type'=>'bool']
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, sold bool )')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, sold bool )');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, sold bool )");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, sold bool )");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_bool = sold
}
```

<!-- end -->

## Timestamps

<!-- example for timestamps  -->

The timestamp type represents Unix timestamps, which are stored as 32-bit integers. Unlike basic integers, the timestamp type allows the use of [time and date](../Functions/Date_and_time_functions.md) functions. Conversion from string values follows these rules:

- Numbers without delimiters, at least 10 characters long, are converted to timestamps as is.
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`
- `%Y-%m-%d`
- `%Y-%m`
- `%Y`

The meanings of these conversion specifiers are detailed in the [strptime manual](https://man7.org/linux/man-pages/man3/strptime.3.html), except for `%E*S`, which stands for milliseconds.

Note that auto-conversion of timestamps is not supported in plain tables.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, date timestamp);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, date timestamp)"
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
utilsApi.sql('CREATE TABLE products(title text, date timestamp)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, date timestamp)');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, date timestamp)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, date timestamp)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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
असल संख्याएँ 32-बिट IEEE 754 एकल सटीकता फ्लोट्स के रूप में संग्रहीत की जाती हैं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, coeff float);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, coeff float)"
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
utilsApi.sql('CREATE TABLE products(title text, coeff float)')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, coeff float)');
```

<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, coeff float)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, coeff float)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

पूर्णांक प्रकारों की तुलना में, समानता के लिए दो फ्लोटिंग-पॉइंट संख्याओं की तुलना की सिफारिश नहीं की जाती है क्योंकि संभावित राउंडिंग त्रुटियाँ हो सकती हैं। एक अधिक विश्वसनीय दृष्टिकोण निर्विवाद त्रुटि सीमा की जाँच करके निकट-बराबर तुलना का उपयोग करना है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select abs(a-b)<=0.00001 from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->
```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"eps":"abs(a-b)"}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"eps":"abs(a-b)"}});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("ebs","abs(a-b)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object>{
    new Dictionary<string, string> { {"ebs", "abs(a-b)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- end -->

<!-- example for float mul -->

एक और विकल्प, जिसका उपयोग `IN(attr,val1,val2,val3)` को निष्पादित करने के लिए किया जा सकता है, फ्लोट्स की तुलना पूर्णांकों के रूप में करना है, एक गुणक कारक चुनकर और ऑपरेशनों में फ्लोट्स को पूर्णांकों में परिवर्तित करके। निम्नलिखित उदाहरण `IN(attr,2.0,2.5,3.5)` को पूर्णांक मूल्यों के साथ काम करने के लिए संशोधित करने को दर्शाता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select in(ceil(attr*100),200,250,350) from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->
```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}}},"expressions":{"inc":"in(ceil(attr*100),200,250,350)"}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("inc","in(ceil(attr*100),200,250,350)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"ebs", "in(ceil(attr*100),200,250,350)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- end -->

<!-- example float_accuracy -->
मैनटिकोर में फ्लोट मान सटीकता के साथ प्रदर्शित किए जाते हैं ताकि वे संग्रहीत मूल्य को सही रूप से दर्शाएँ। इस दृष्टिकोण को सटीकता हानि को रोकने के लिए पेश किया गया था, विशेष रूप से भौगोलिक निर्देशांकों जैसे मामलों के लिए, जहाँ 6 दशमलव स्थानों तक गोल करने से गलतियाँ होती थीं।
अब, मैन्टिकोर पहले 6 अंकों की संख्या आउटपुट करता है, फिर इसे मूल मूल्य के साथ पार्स और तुलना करता है। अगर वे मेल नहीं खाते, तो अतिरिक्त अंक जोड़े जाते हैं जब तक कि वे मेल नहीं खाते। 

उदाहरण के लिए, यदि एक फ्लोट मान `19.45` के रूप में डाला गया था, तो मैन्टिकोर इसे `19.450001` के रूप में प्रदर्शित करेगा ताकि संग्रहीत मान को सटीक रूप से दर्शाया जा सके। 

<!-- request Example -->
```sql
insert into t(id, f) values(1, 19.45)
--------------

Query OK, 1 row affected (0.02 sec)

--------------
select * from t
--------------

+------+-----------+
| id   | f         |
+------+-----------+
|    1 | 19.450001 |
+------+-----------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

<!-- end -->


## JSON

<!-- example for creating json -->

यह डेटा प्रकार JSON वस्तुओं के भंडारण की अनुमति देता है, जो स्कीमा-लेस डेटा को संभालने के लिए विशेष रूप से उपयोगी है। जब JSON मानों को परिभाषित करते हैं, तो सुनिश्चित करें कि वस्तुओं के लिए खोलने वाली और बंद करने वाली कर्ली ब्रैस `{` और `}` शामिल हैं, या Arrays के लिए स्क्वायर ब्रैकेट `[` और `]` हैं। जबकि JSON स्तंभीय भंडारण द्वारा समर्थित नहीं है, इसे पारंपरिक पंक्ति-वार भंडारण में संग्रहीत किया जा सकता है। यह ध्यान देने योग्य है कि दोनों भंडारण प्रकारों को एक ही तालिका के भीतर जोड़ा जा सकता है। 

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, data json);
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, data json)"
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
utilsApi.sql('CREATE TABLE products(title text, data json)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, data json)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql'CREATE TABLE products(title text, data json)');
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql'CREATE TABLE products(title text, data json)');
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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

JSON गुणों का उपयोग अधिकांश संचालन में किया जा सकता है। विशेष कार्य भी हैं जैसे [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29), [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29), [GREATEST()](../Functions/Mathematical_functions.md#GREATEST%28%29), [LEAST()](../Functions/Mathematical_functions.md#LEAST%28%29) और [INDEXOF()](../Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29) जो प्रॉपर्टी Arrays की यात्रा की अनुमति देते हैं। 

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select indexof(x>2 for x in data.intarray) from products
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}},"expressions":{"idx":"indexof(x>2 for x in data.intarray)"}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("idx","indexof(x>2 for x in data.intarray)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"idx", "indexof(x>2 for x in data.intarray)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- end -->

<!-- example for REGEX() json -->

टेक्स्ट गुणों को स्ट्रिंग की तरह ही माना जाता है, इसलिए उन्हें पूर्ण-पाठ मिलान अभिव्यक्तियों में उपयोग करना संभव नहीं है। हालाँकि, स्ट्रिंग कार्य जैसे [REGEX()](../Functions/String_functions.md#REGEX%28%29) का उपयोग किया जा सकता है। 

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select regex(data.name, 'est') as c from products where c>0
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"range":{"c":{"gt":0}}}},"expressions":{"c":"regex(data.name, 'est')"}}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("range", new HashMap<String,Object>(){{
    put("c", new HashMap<String,Object>(){{
        put("gt",0);
    }});
}});
searchRequest.setQuery(query);
Object expressions = new HashMap<String,Object>(){{
    put("idx","indexof(x>2 for x in data.intarray)");
}};
searchRequest.setExpressions(expressions);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
var rangeFilter = new RangeFilter("c");
rangeFilter.Gt = 0;
searchRequest.AttrFilter = rangeFilter;
searchRequest.Expressions = new List<Object> {
    new Dictionary<string, string> { {"idx", "indexof(x>2 for x in data.intarray)"} }
};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- end -->

<!-- example for DOUBLE() -->

In the case of JSON properties, enforcing data type may be required for proper functionality in certain situations. For example, when working with float values, [DOUBLE()](../Functions/Type_casting_functions.md#DOUBLE%28%29) must be used for proper sorting.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products order by double(data.myfloat) desc
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{}}},"sort":[{"double(data.myfloat)":{"order":"desc"}}]});
```
<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("double(data.myfloat)",new HashMap<String,String>(){{ put("order","desc");}});}});
}});
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Sort = new List<Object> {
    new SortOrder("double(data.myfloat)", SortOrder.OrderEnum.Desc)
};
var searchResponse = searchApi.Search(searchRequest);
```

<!-- end -->

## Float vector

<!-- example for creating float_vector -->
Float vector attributes allow storing variable-length lists of floats, primarily used for machine learning applications and similarity searches. This type differs from multi-valued attributes (MVAs) in several important ways:
- Preserves the exact order of values (unlike MVAs which may reorder)
- Retains duplicate values (unlike MVAs which deduplicate)
- No additional processing during insertion (unlike MVAs which sort and deduplicate)

### Usage and Limitations
- Currently only supported in real-time tables
- Can only be utilized in KNN (k-nearest neighbor) searches
- Not supported in plain tables or other functions/expressions
- When used with KNN settings, you cannot `UPDATE` `float_vector` values. Use `REPLACE` instead
- When used without KNN settings, you can `UPDATE` `float_vector` values
- Float vectors cannot be used in regular filters or sorting
- The only way to filter by `float_vector` values is through vector search operations (KNN)

### Common Use Cases
- Image embeddings for similarity search
- Text embeddings for semantic search
- Feature vectors for machine learning
- Recommendation system vectors

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, image_vector float_vector);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, image_vector float_vector)"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
	'image_vector'=>['type'=>'float_vector']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, image_vector float_vector)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, image_vector float_vector)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, image_vector float_vector)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, image_vector float_vector)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_float_vector = image_vector
}
```

<!-- end -->

For information about using float vectors in searches, see [KNN search](../Searching/KNN.md).

## Multi-value integer (MVA)

<!-- example for creating MVA32 -->

Multi-value attributes allow storing variable-length lists of 32-bit unsigned integers. This can be useful for storing one-to-many numeric values, such as tags, product categories, and properties.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, product_codes multi);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, product_codes multi)"
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
utilsApi.sql('CREATE TABLE products(title text, product_codes multi)')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, product_codes multi)');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, product_codes multi)");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, product_codes multi)");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
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
It supports filtering and aggregation, but not sorting. Filtering can be done using a condition that requires at least one element to pass (using [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)) or all elements ([ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29)) to pass.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select * from products where any(product_codes)=3
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
  "query":
  {
    "match_all": {},
    "equals" : { "any(product_codes)": 3 }
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
searchApi.search({"table":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"equals":{"any(product_codes)":3}}}})'
```
<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
query.put("equals",new HashMap<String,Integer>(){{
     put("any(product_codes)",3);
}});
searchRequest.setQuery(query);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.AttrFilter = new EqualsFilter("any(product_codes)", 3);
var searchResponse = searchApi.Search(searchRequest);
```
<!-- end -->

<!-- example for least/greatest MVA -->

Information like [least](../Functions/Mathematical_functions.md#LEAST%28%29) or [greatest](../Functions/Mathematical_functions.md#GREATEST%28%29) element and length of the list can be extracted. An example shows ordering by the least element of a multi-value attribute.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
select least(product_codes) l from products order by l asc
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "products",
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
searchApi.search({"table":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"match_all":{},"sort":[{"product_codes":{"order":"asc","mode":"min"}}]}});
```

<!-- intro -->
##### java:

<!-- request java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setSort(new ArrayList<Object>(){{
    add(new HashMap<String,String>(){{ put("product_codes",new HashMap<String,String>(){{ put("order","asc");put("mode","min");}});}});
}});
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
object query =  new { match_all=null };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Sort = new List<Object> {
    new SortMVA("product_codes", SortOrder.OrderEnum.Asc, SortMVA.ModeEnum.Min)
};
searchResponse = searchApi.Search(searchRequest);
```

<!-- end -->

<!-- example for grouping by MVA -->
When grouping by a multi-value attribute, a document will contribute to as many groups as there are different values associated with that document. For instance, if a collection contains exactly one document having a 'product_codes' multi-value attribute with values 5, 7, and 11, grouping on 'product_codes' will produce 3 groups with `COUNT(*)`equal to 1 and `GROUPBY()` key values of 5, 7, and 11, respectively. Also, note that grouping by multi-value attributes may lead to duplicate documents in the result set because each document can participate in many groups.

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
The order of the numbers inserted as values of multivalued attributes is not preserved. Values are stored internally as a sorted set.

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
##### JSON:

<!-- request JSON -->

```JSON
POST /insert
{
	"table":"products",
	"id":1,
	"doc":
	{
		"title":"first",
		"product_codes":[4,2,1,3]
	}
}

POST /search
{
  "table": "products",
  "query": { "match_all": {} }
}
```

<!-- response JSON -->

```JSON
{
   "table":"products",
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
            "_id": 1,
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
indexApi.insert({"table":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}})
searchApi.search({"table":"products","query":{"match_all":{}}})
```
<!-- response Python -->

```python
{'created': True,
 'found': None,
 'id': 1,
 'table': 'products',
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
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
await indexApi.insert({"table":"products","id":1,"doc":{"title":"first","product_codes":[4,2,1,3]}});
res = await searchApi.search({"table":"products","query":{"match_all":{}}});
```
<!-- response javascript -->

```javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1,"_source":{"product_codes":[1,2,3,4],"title":"first"}}]}}
```
<!-- intro -->
##### java:

<!-- request java -->

```java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","first");
    put("product_codes",new int[] {4,2,1,3});
}};
newdoc.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("products");
searchRequest.setQuery(query);
SearchResponse searchResponse = searchApi.search(searchRequest);
System.out.println(searchResponse.toString() );
```
<!-- response java -->

```java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=1, _score=1, _source={product_codes=[1, 2, 3, 4], title=first}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "first");
doc.Add("product_codes", new List<Object> {4,2,1,3});
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
object query =  new { match_all=null };
var searchRequest = new SearchRequest("products", query);
var searchResponse = searchApi.Search(searchRequest);
Console.WriteLine(searchResponse.ToString())
```
<!-- response C# -->

```clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=1, _score=1, _source={product_codes=[1, 2, 3, 4], title=first}}]
        aggregations: null
    }
    profile: null
}

```

<!-- end -->


## Multi-value big integer

<!-- example for creating MVA64 -->

A data type that allows storing variable-length lists of 64-bit signed integers. It has the same functionality as multi-value integer.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, values multi64);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, values multi64)"
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
utilsApi.sql('CREATE TABLE products(title text, values multi64))')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, values multi64))');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, values multi64))");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, values multi64))");
```

<!-- intro -->
##### config:

<!-- request config -->

```ini
table products
{
	type = rt
	path = products

	rt_field = title
	stored_fields = title

	rt_attr_multi_64 = values
}
```

<!-- end -->

## Columnar attribute properties

When you use the columnar storage you can specify the following properties for the attributes.

<!-- example fast_fetch -->
### fast_fetch

By default, Manticore Columnar storage stores all attributes in a columnar fashion, as well as in a special docstore row by row. This enables fast execution of queries like `SELECT * FROM ...`, especially when fetching a large number of records at once. However, if you are sure that you do not need it or wish to save disk space, you can disable it by specifying `fast_fetch='0'` when creating a table or (if you are defining a table in a config) by using `columnar_no_fast_fetch` as shown in the following example.

<!-- request RT mode -->
```sql
create table t(a int, b int fast_fetch='0') engine='columnar'; desc t;
```

<!-- response RT mode -->

```sql
+-------+--------+---------------------+
| Field | Type   | Properties          |
+-------+--------+---------------------+
| id    | bigint | columnar fast_fetch |
| a     | uint   | columnar fast_fetch |
| b     | uint   | columnar            |
+-------+--------+---------------------+
3 rows in set (0.00 sec)
```

<!-- request Plain mode -->

```ini
source min {
    type = mysql
    sql_host = localhost
    sql_user = test
    sql_pass =
    sql_db = test
    sql_query = select 1, 1 a, 1 b
    sql_attr_uint = a
    sql_attr_uint = b
}

table tbl {
    path = tbl/col
    source = min
    columnar_attrs = *
    columnar_no_fast_fetch = b
}
```

<!-- response Plain mode -->

```sql
+-------+--------+---------------------+
| Field | Type   | Properties          |
+-------+--------+---------------------+
| id    | bigint | columnar fast_fetch |
| a     | uint   | columnar fast_fetch |
| b     | uint   | columnar            |
+-------+--------+---------------------+
```

<!-- end -->
<!-- proofread -->






