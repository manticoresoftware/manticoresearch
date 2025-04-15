# परकोलाट टेबल में नियम जोड़ना

<!-- example -->
एक [परकोलाट टेबल](../../Creating_a_table/Local_tables/Percolate_table.md) में दस्तावेज़ जो परकोलाट क्वेरी नियमों को संग्रहित करते हैं और चार फ़ील्ड के सटीक स्कीमा का पालन करना आवश्यक है:

| फ़ील्ड | प्रकार | विवरण |
| - | - | - |
| id | bigint | PQ नियम पहचानकर्ता (अगर छोड़ दिया गया, तो इसे स्वचालित रूप से असाइन किया जाएगा) |
| क्वेरी | string | पूर्ण-पाठ क्वेरी (खाली हो सकती है) [परकोलाट टेबल](../../Creating_a_table/Local_tables/Percolate_table.md) के साथ संगत |
| फ़िल्टर्स | string | गैर-पूर्ण-पाठ फ़ील्ड द्वारा अतिरिक्त फ़िल्टर (खाली हो सकते हैं) [परकोलाट टेबल](../../Creating_a_table/Local_tables/Percolate_table.md) के साथ संगत |
| टैग्स   | string | एक या कई अल्पविराम से अलग टैग वाली स्ट्रिंग, जिसका उपयोग संग्रहीत प्रश्नों को चयनात्मक रूप से प्रदर्शित/हटाने के लिए किया जा सकता है |

अन्य कोई फ़ील्ड नाम समर्थित नहीं हैं और एक त्रुटि उत्पन्न करेंगे।

**चेतावनी:** SQL के माध्यम से JSON-फॉर्मेटेड PQ नियम डालना/बदलना काम नहीं करेगा। दूसरे शब्दों में, JSON-विशिष्ट ऑपरेटर (`match` आदि) केवल नियम के पाठ के हिस्से माने जाएंगे जिन्हें दस्तावेज़ों के साथ मेल खाना चाहिए। यदि आप JSON सिंटैक्स पसंद करते हैं, तो `INSERT`/`REPLACE` के बजाय HTTP एंडपॉइंट का उपयोग करें।

<!-- intro -->
##### SQL
<!-- request SQL -->

```sql
INSERT INTO pq(id, query, filters) VALUES (1, '@title shoes', 'price > 5');
INSERT INTO pq(id, query, tags) VALUES (2, '@title bag', 'Louis Vuitton');
SELECT * FROM pq;
```
<!-- response SQL -->

```sql
+------+--------------+---------------+---------+
| id   | query        | tags          | filters |
+------+--------------+---------------+---------+
|    1 | @title shoes |               | price>5 |
|    2 | @title bag   | Louis Vuitton |         |
+------+--------------+---------------+---------+
```
<!-- intro -->
##### JSON
<!-- request JSON -->
आप एक परकोलाट क्वेरी को परकोलाट टेबल में जोड़ने के लिए दो तरीके अपना सकते हैं:
* JSON /search संगत प्रारूप में क्वेरी, जो [json/search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) में वर्णित है
```json
PUT /pq/pq_table/doc/1
{
  "query": {
    "match": {
      "title": "shoes"
    },
    "range": {
      "price": {
        "gt": 5
      }
    }
  },
  "tags": ["Loius Vuitton"]
}
```

* SQL प्रारूप में क्वेरी, जो [search query syntax](../../Searching/Filters.md#Queries-in-SQL-format) में वर्णित है
```json
PUT /pq/pq_table/doc/2
{
  "query": {
    "ql": "@title shoes"
  },
  "filters": "price > 5",
  "tags": ["Loius Vuitton"]
}
```
<!-- intro -->
##### PHP
<!-- request PHP -->
```php
$newstoredquery = [
    'table' => 'test_pq',
    'body' => [
        'query' => [
                       'match' => [
                               'title' => 'shoes'
                       ]
               ],
               'range' => [
                       'price' => [
                               'gt' => 5
                       ]
               ]
       ],
    'tags' => ['Loius Vuitton']
];
$client->pq()->doc($newstoredquery);
```

<!-- intro -->
##### Python
<!-- request Python -->
```python
newstoredquery ={"table" : "test_pq", "id" : 2, "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}}
indexApi.insert(newstoredquery)
```

<!-- intro -->
##### Javascript
<!-- request Javascript -->
```javascript
newstoredquery ={"table" : "test_pq", "id" : 2, "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}};
indexApi.insert(newstoredquery);
```
<!-- intro -->
##### java
<!-- request Java -->
```java
newstoredquery = new HashMap<String,Object>(){{
    put("query",new HashMap<String,Object >(){{
        put("q1","@title shoes");
        put("filters","price>5");
        put("tags",new String[] {"Loius Vuitton"});
    }});
}};
newdoc.index("test_pq").id(2L).setDoc(doc);
indexApi.insert(newdoc);
```

<!-- intro -->
##### C#
<!-- request C# -->
```clike
Dictionary<string, Object> query = new Dictionary<string, Object>(); 
query.Add("q1", "@title shoes");
query.Add("filters", "price>5");
query.Add("tags", new List<string> {"Loius Vuitton"});
Dictionary<string, Object> newstoredquery = new Dictionary<string, Object>(); 
newstoredquery.Add("query", query);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "test_pq", id: 2, doc: doc);
indexApi.Insert(newdoc);
```

<!-- end -->

<!-- example noid -->
## ऑटो ID प्रावधान

यदि आप एक ID निर्दिष्ट नहीं करते हैं, तो इसे स्वचालित रूप से असाइन किया जाएगा। आप ऑटो-ID के बारे में अधिक पढ़ सकते हैं [यहाँ](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID)।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
INSERT INTO pq(query, filters) VALUES ('wristband', 'price > 5');
SELECT * FROM pq;
```
<!-- response SQL -->

```sql
+---------------------+-----------+------+---------+
| id                  | query     | tags | filters |
+---------------------+-----------+------+---------+
| 1657843905795719192 | wristband |      | price>5 |
+---------------------+-----------+------+---------+
```
<!-- intro -->
##### JSON
<!-- request JSON -->

```json
PUT /pq/pq_table/doc
{
"query": {
  "match": {
    "title": "shoes"
  },
  "range": {
    "price": {
      "gt": 5
    }
  }
},
"tags": ["Loius Vuitton"]
}

PUT /pq/pq_table/doc
{
"query": {
  "ql": "@title shoes"
},
"filters": "price > 5",
"tags": ["Loius Vuitton"]
}
```
<!-- response JSON -->

```json
{
  "table": "pq_table",
  "type": "doc",
  "_id": 1657843905795719196,
  "result": "created"
}

{
  "table": "pq_table",
  "type": "doc",
  "_id": 1657843905795719198,
  "result": "निर्मित"
}
```
<!-- intro -->
##### PHP
<!-- request PHP -->
```php
$newstoredquery = [
    'table' => 'pq_table',
    'body' => [
        'query' => [
                       'match' => [
                               'title' => 'जूते'
                       ]
               ],
               'range' => [
                       'price' => [
                               'gt' => 5
                       ]
               ]
       ],
    'tags' => ['Loius Vuitton']
];
$client->pq()->doc($newstoredquery);
```
<!-- response PHP -->
```php
Array(
       [index] => pq_table
       [type] => doc
       [_id] => 1657843905795719198
       [result] => निर्मित
)
```

<!-- intro -->
##### Python
<!-- request Python -->
```python
indexApi = api = manticoresearch.IndexApi(client)
newstoredquery ={"table" : "test_pq",   "doc" : {"query": {"ql": "@title जूते"},"filters": "price > 5","tags": ["Loius Vuitton"]}}
indexApi.insert(store_query)
```
<!-- response Python -->
```python
{'created': True,
 'found': None,
 'id': 1657843905795719198,
 'table': 'test_pq',
 'result': 'निर्मित'}
```
<!-- intro -->
##### Javascript
<!-- request Javascript -->
```javascript
newstoredquery ={"table" : "test_pq",  "doc" : {"query": {"ql": "@title जूते"},"filters": "price > 5","tags": ["Loius Vuitton"]}};
res =  await indexApi.insert(store_query);
```
<!-- response Javascript -->
```javascript
{"table":"test_pq","_id":1657843905795719198,"created":true,"result":"निर्मित"}

```

<!-- intro -->
##### java
<!-- request Java -->
```java
newstoredquery = new HashMap<String,Object>(){{
    put("query",new HashMap<String,Object >(){{
        put("q1","@title जूते");
        put("filters","price>5");
        put("tags",new String[] {"Loius Vuitton"});
    }});
}};
newdoc.index("test_pq").setDoc(doc);
indexApi.insert(newdoc);
```

<!-- intro -->
##### C#
<!-- request C# -->
```clike
Dictionary<string, Object> query = new Dictionary<string, Object>(); 
query.Add("q1", "@title जूते");
query.Add("filters", "price>5");
query.Add("tags", new List<string> {"Loius Vuitton"});
Dictionary<string, Object> newstoredquery = new Dictionary<string, Object>(); 
newstoredquery.Add("query", query);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "test_pq", doc: doc);
indexApi.Insert(newdoc);
```

<!-- end -->

<!-- example noschema -->
## SQL में कोई स्कीमा नहीं
SQL `INSERT` कमांड में स्कीमा छोड़ने की स्थिति में, निम्नलिखित पैरामीटर अपेक्षित हैं:
1. ID. आप ऑटो-ID निर्माण को सक्रिय करने के लिए `0` का उपयोग कर सकते हैं।
2. क्वेरी - पूर्ण-ग्रंथ क्वेरी।
3. टैग - PQ नियम टैग स्ट्रिंग।
4. फ़िल्टर - गुणों द्वारा अतिरिक्त फ़िल्टर।

<!-- request SQL -->

```sql
INSERT INTO pq VALUES (0, '@title जूते', '', '');
INSERT INTO pq VALUES (0, '@title जूते', 'Louis Vuitton', '');
SELECT * FROM pq;
```
<!-- response SQL -->

```sql
+---------------------+--------------+---------------+---------+
| id                  | query        | tags          | filters |
+---------------------+--------------+---------------+---------+
| 2810855531667783688 | @title जूते |               |         |
| 2810855531667783689 | @title जूते | Louis Vuitton |         |
+---------------------+--------------+---------------+---------+
```
<!-- end -->

<!-- example replace -->
## किसी PQ तालिका में नियमों को प्रतिस्थापित करना

SQL में नए नियम के साथ मौजूदा PQ नियम को प्रतिस्थापित करने के लिए, बस एक नियमित [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) कमांड का उपयोग करें। HTTP JSON इंटरफेस के माध्यम से **JSON मोड में परिभाषित** PQ नियम को प्रतिस्थापित करने के लिए एक विशेष वाक्यिका `?refresh=1` है।


<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
mysql> select * from pq;
+---------------------+--------------+------+---------+
| id                  | query        | tags | filters |
+---------------------+--------------+------+---------+
| 2810823411335430148 | @title जूते |      |         |
+---------------------+--------------+------+---------+
1 row in set (0.00 sec)

mysql> replace into pq(id,query) values(2810823411335430148,'@title बूट');
Query OK, 1 row affected (0.00 sec)

mysql> select * from pq;
+---------------------+--------------+------+---------+
| id                  | query        | tags | filters |
+---------------------+--------------+------+---------+
| 2810823411335430148 | @title बूट |      |         |
+---------------------+--------------+------+---------+
1 row in set (0.00 sec)
```

<!-- request JSON -->
```json
GET /pq/pq/doc/2810823411335430149
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "_id": 2810823411335430149,
        "_score": 1,
        "_source": {
          "query": {
            "match": {
              "title": "जूते"
            }
          },
          "tags": "",
          "filters": ""
        }
      }
    ]
  }
}

PUT /pq/pq/doc/2810823411335430149?refresh=1 -d '{
  "query": {
    "match": {
      "title": "बूट"
    }
  }
}'

GET /pq/pq/doc/2810823411335430149
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "_id": 2810823411335430149,
        "_score": 1,
        "_source": {
          "query": {
            "match": {
              "title": "बूट"
            }
          },
          "tags": "",
          "filters": ""
        }
      }
    ]
  }
}
```

<!-- end -->
<!-- proofread -->
