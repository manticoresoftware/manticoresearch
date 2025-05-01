# त्वरित प्रारंभ मार्गदर्शिका

<!-- example install -->
## स्थापित करें और मैन्टिकॉर प्रारंभ करें

आप विभिन्न ऑपरेटिंग सिस्टमों पर आसानी से मैन्टिकॉर स्थापित और प्रारंभ कर सकते हैं, जिनमें उबंटू, सेंटोस, डेबियन, विंडोज, और मैकोएस शामिल हैं। इसके अतिरिक्त, आप डॉकर कंटेनर के रूप में भी मैन्टिकॉर का उपयोग कर सकते हैं।

<!-- intro -->
### उबंटू

<!-- request Ubuntu -->
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### डेबियन

<!-- request Debian -->
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### सेंटोस

<!-- request Centos -->
```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### विंडोज

<!-- request Windows -->
* https://manticoresearch.com/install/ से विंडोज संग्रह डाउनलोड करें
* संग्रह से सभी फाइलों को `C:Manticore` में निकालें
* मैन्टिकॉर को सेवा के रूप में स्थापित करने के लिए निम्नलिखित कमांड चलाएं:
* ```bash
  C:Manticoreinsearchd --install --config C:Manticoresphinx.conf.in --servicename Manticore
  ```
* माइक्रोसॉफ्ट प्रबंधन कंसोल के सेवाएँ स्नैप-इन से मैन्टिकॉर प्रारंभ करें।

<!-- intro -->
### मैकोएस

<!-- request MacOS -->
```bash
brew install manticoresearch
brew services start manticoresearch
```

<!-- intro -->
### डॉकर
<!-- request Docker -->
```bash
docker pull manticoresearch/manticore
docker run --name manticore -p9306:9306 -p9308:9308 -p9312:9312 -d manticoresearch/manticore
```
अपने डेटा निर्देशिका को बनाए रखने के लिए, पढ़ें [उत्पादन में मैन्टिकॉर डॉकर का उपयोग कैसे करें](Starting_the_server/Docker.md#Production-use)
<!-- end -->

<!-- example connect -->
## मैन्टिकॉर से कनेक्ट करें

डिफ़ॉल्ट रूप से मैन्टिकॉर आपके कनेक्शन के लिए प्रतीक्षा कर रहा है:

  * MySQL क्लाइंट के लिए पोर्ट 9306
  * HTTP/HTTPS कनेक्शनों के लिए पोर्ट 9308
  * अन्य मैन्टिकॉर नोड और मैन्टिकॉर बायनरी एपीआई पर आधारित क्लाइंट से कनेक्शनों के लिए पोर्ट 9312

HTTPS समर्थन के बारे में अधिक जानकारी हमारे शिक्षण पाठ्यक्रम में [यहाँ](https://play.manticoresearch.com/https/) पाई जा सकती है।

<!-- intro -->
##### MySQL के माध्यम से कनेक्ट करें:

<!-- request SQL -->
```bash
mysql -h0 -P9306
```

<!-- intro -->
##### HTTP के माध्यम से JSON के माध्यम से कनेक्ट करें

<!-- request HTTP -->
HTTP एक राज्यहीन प्रोटोकॉल है, इसलिए इसे किसी विशेष कनेक्शन चरण की आवश्यकता नहीं होती। आप बस सर्वर को एक HTTP अनुरोध भेज सकते हैं और प्रतिक्रिया प्राप्त कर सकते हैं। मैन्टिकॉर के साथ JSON इंटरफेस का उपयोग करके संचार करने के लिए, आप अपनी पसंद की प्रोग्रामिंग भाषा में किसी भी HTTP क्लाइंट पुस्तकालय का उपयोग करके GET या POST अनुरोध भेज सकते हैं और JSON प्रतिक्रियाओं को पार्स कर सकते हैं:

```bash
curl -s "http://localhost:9308/search"
```

<!-- intro -->
##### [PHP क्लाइंट](https://github.com/manticoresoftware/manticoresearch-php) के माध्यम से कनेक्ट करें:

<!-- request PHP -->
```php
// https://github.com/manticoresoftware/manticoresearch-php
require_once __DIR__ . '/vendor/autoload.php';
$config = ['host'=>'127.0.0.1','port'=>9308];
$client = new ManticoresearchClient($config);
```

<!-- intro -->
##### [Python क्लाइंट](https://github.com/manticoresoftware/manticoresearch-php) के माध्यम से कनेक्ट करें:

<!-- request Python -->
```python
// https://github.com/manticoresoftware/manticoresearch-python
import manticoresearch
config = manticoresearch.Configuration(
    host = "http://127.0.0.1:9308"
)
client = manticoresearch.ApiClient(config)
indexApi = manticoresearch.IndexApi(client)
searchApi = manticoresearch.SearchApi(client)
utilsApi = manticoresearch.UtilsApi(client)
```

<!-- intro -->
##### [Python asyncio क्लाइंट](https://github.com/manticoresoftware/manticoresearch-php-asyncio) के माध्यम से कनेक्ट करें:

<!-- request Python-asyncio -->
```python
// https://github.com/manticoresoftware/manticoresearch-python-asyncio
import manticoresearch
config = manticoresearch.Configuration(
    host = "http://127.0.0.1:9308"
)
async with manticoresearch.ApiClient(config) as client:
    indexApi = manticoresearch.IndexApi(client)
    searchApi = manticoresearch.searchApi(client)
    utilsApi = manticoresearch.UtilsApi(client)
```

<!-- intro -->
##### [Javascript क्लाइंट](https://github.com/manticoresoftware/manticoresearch-javascript) के माध्यम से कनेक्ट करें:

<!-- request Javascript -->
```javascript
// https://github.com/manticoresoftware/manticoresearch-javascript
var Manticoresearch = require('manticoresearch');
var client= new Manticoresearch.ApiClient()
client.basePath="http://127.0.0.1:9308";
indexApi = new Manticoresearch.IndexApi(client);
searchApi = new Manticoresearch.SearchApi(client);
utilsApi = new Manticoresearch.UtilsApi(client);
```
<!-- intro -->
##### [Java क्लाइंट](https://github.com/manticoresoftware/manticoresearch-java) के माध्यम से कनेक्ट करें:

<!-- request Java -->
```java
// https://github.com/manticoresoftware/manticoresearch-java
import com.manticoresearch.client.*;
import com.manticoresearch.client.model.*;
import com.manticoresearch.client.api.*;
...
ApiClient client = Configuration.getDefaultApiClient();
client.setBasePath("http://127.0.0.1:9308");
...
IndexApi indexApi = new IndexApi(client);
SearchApi searchApi = new UtilsApi(client);
UtilsApi utilsApi = new UtilsApi(client);
```

<!-- intro -->
##### [.Net क्लाइंट](https://github.com/manticoresoftware/manticoresearch-net) के माध्यम से कनेक्ट करें:

<!-- request C# -->
```clike
// https://github.com/manticoresoftware/manticoresearch-net
using System.Net.Http;
...
using ManticoreSearch.Client;
using ManticoreSearch.Api;
using ManticoreSearch.Model;
...
config = new Configuration();
config.BasePath = "http://localhost:9308";
httpClient = new HttpClient();
httpClientHandler = new HttpClientHandler();
...
var indexApi = new IndexApi(httpClient, config, httpClientHandler);
var searchApi = new SearchApi(httpClient, config, httpClientHandler);
var utilsApi = new UtilsApi(httpClient, config, httpClientHandler);
```

<!-- intro -->
##### [Rust client](https://github.com/manticoresoftware/manticoresearch-rust) के माध्यम से कनेक्ट करें:

<!-- request Rust -->
```rust
// https://github.com/manticoresoftware/manticoresearch-rust
use std::sync::Arc;
use manticoresearch::{
    apis::{
        {configuration::Configuration,IndexApi,IndexApiClient,SearchApi,SearchApiClient,UtilsApi,UtilsApiClient}
    },
};

async fn maticore_connect {

let configuration = Configuration {

  base_path: "http://127.0.0.1:9308".to_owned(),

  ..Default::default(),

};
    let api_config = Arc::new(configuration);
    let utils_api = UtilsApiClient::new(api_config.clone());
    let index_api = IndexApiClient::new(api_config.clone());
    let search_api = SearchApiClient::new(api_config.clone());
```

<!-- intro -->
##### [TypeScript client](https://github.com/manticoresoftware/manticoresearch-typescript) के माध्यम से कनेक्ट करें:

<!-- request Typescript -->
```typescript
import {
  Configuration,
  IndexApi,
  SearchApi,
  UtilsApi
} from "manticoresearch-ts";
...
const config = new Configuration({
  basePath: 'http://localhost:9308',
})
const indexApi = new IndexApi(config);
const searchApi = new SearchApi(config);
const utilsApi = new UtilsApi(config);
```

<!-- intro -->
##### [Go client](https://github.com/manticoresoftware/manticoresearch-go) के माध्यम से कनेक्ट करें:

<!-- request Go -->
```go
import (

"context"

manticoreclient "github.com/manticoresoftware/manticoresearch-go"
)
...
configuration := manticoreclient.NewConfiguration()
configuration.Servers[0].URL = "http://localhost:9308"
apiClient := manticoreclient.NewAPIClient(configuration)
```

<!-- end -->

<!-- example create -->
## एक तालिका बनाएं

आइए अब "products" नामक एक तालिका बनाते हैं जिसमें 2 फ़ील्ड हैं:
* title - पूर्ण-टेक्स्ट फ़ील्ड जो हमारे उत्पाद के शीर्षक को समाहित करेगा
* price - "float" प्रकार का

ध्यान दें कि एक स्पष्ट निर्माण कथन के साथ तालिका बनाना छोड़ना संभव है। अधिक जानकारी के लिए [Auto schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) देखें।

तालिका बनाने के विभिन्न तरीकों के बारे में अधिक जानकारी हमारे शिक्षण पाठ्यक्रमों में उपलब्ध है:
* [एक RealTime तालिका बनाना](https://play.manticoresearch.com/rtmode/)
* [MySQL स्रोत से तालिका बनाना](https://play.manticoresearch.com/mysql/)
* [CSV स्रोत से तालिका बनाना](https://play.manticoresearch.com/csv/)
* [ऑटो स्कीमा तंत्र का उपयोग करके तालिका बनाना](https://play.manticoresearch.com/autoschema/)
* [Logstash/Beats का उपयोग करके तालिका बनाना](https://play.manticoresearch.com/logstash/)
* [Fluentbit का उपयोग करके तालिका बनाना](https://play.manticoresearch.com/vectordev/)
* [Vector.dev एजेंट का उपयोग करके तालिका बनाना](https://play.manticoresearch.com/vectordev/)

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text, price float) morphology='stem_en';
```
<!-- response -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```bash
POST /cli -d "create table products(title text, price float) morphology='stem_en'"
```

<!-- response HTTP -->
```json
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
    'price'=>['type'=>'float'],
],['morphology' => 'stem_en']);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('create table products(title text, price float) morphology='stem_en'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('create table products(title text, price float) morphology='stem_en'')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('create table products(title text, price float) morphology='stem_en'');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("create table products(title text, price float) morphology='stem_en'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("create table products(title text, price float) morphology='stem_en'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("create table products(title text, price float) morphology='stem_en'", Some(true)).await;
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
```typescript
res = await utilsApi.sql('create table products(title text, price float) morphology='stem_en'');
```

<!-- intro -->
##### Go:

<!-- request Go -->
```go
res := apiClient.UtilsAPI.Sql(context.Background()).Body("create table products(title text, price float) morphology='stem_en'").Execute();
```

<!-- end -->

<!-- example insert -->
## दस्तावेज़ जोड़ें

आइए अब तालिका में कुछ दस्तावेज़ जोड़ें:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
उत्पादों में डालें(title,मूल्य) मान ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
प्रश्न OK, 3 पंक्तियाँ प्रभावित (0.01 सेकंड)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->
`"id":0` या कोई id स्वचालित ID पीढ़ी को मजबूर करता है।

```json
POST /insert
{
  "table":"products",
  "doc":
  {
    "title" : "Crossbody Bag with Tassel",
    "price" : 19.85
  }
}


POST /insert
{
  "table":"products",
  "doc":
  {
    "title" : "microfiber sheet set",
    "price" : 19.99
  }
}

POST /insert
{
  "table":"products",
  "doc":
  {
    "title" : "Pet Hair Remover Glove",
    "price" : 7.99
  }
}
```
<!-- response JSON -->

```json
{
  "table": "products",
  "_id": 0,
  "created": true,
  "result": "created",
  "status": 201
}

{
  "table": "products",
  "_id": 0,
  "created": true,
  "result": "created",
  "status": 201
}

{
  "table": "products",
  "_id": 0,
  "created": true,
  "result": "created",
  "status": 201
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->addDocuments([
        ['title' => 'Crossbody Bag with Tassel', 'price' => 19.85],
        ['title' => 'microfiber sheet set', 'price' => 19.99],
        ['title' => 'Pet Hair Remover Glove', 'price' => 7.99]
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

``` python
indexApi.insert({"table" : "products", "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}})
indexApi.insert({"table" : "products", "doc" : {"title" : "microfiber sheet set", "price" : 19.99}})
indexApi.insert({"table" : "products", "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

``` python
await indexApi.insert({"table" : "products", "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}})
await indexApi.insert({"table" : "products", "doc" : {"title" : "microfiber sheet set", "price" : 19.99}})
await indexApi.insert({"table" : "products", "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}});
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "microfiber sheet set", "price" : 19.99}});
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Crossbody Bag with Tassel");
    put("price",19.85);
}};
newdoc.index("products").setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
doc = new HashMap<String,Object>(){{
    put("title","microfiber sheet set");
    put("price",19.99);
}};
newdoc.index("products").setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
doc = new HashMap<String,Object>(){{
    put("title","Pet Hair Remover Glove");
    put("price",7.99);
 }};
newdoc.index("products").setDoc(doc);
indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title","Crossbody Bag with Tassel");
doc.Add("price",19.85);
InsertDocumentRequest insertDocumentRequest = new InsertDocumentRequest(index: "products", doc: doc);
sqlresult = indexApi.Insert(insertDocumentRequest);

doc = new Dictionary<string, Object>();
doc.Add("title","microfiber sheet set");
doc.Add("price",19.99);
insertDocumentRequest = new InsertDocumentRequest(index: "products", doc: doc);
sqlresult = indexApi.Insert(insertDocumentRequest);

doc = new Dictionary<string, Object>();
doc.Add("title","Pet Hair Remover Glove");
doc.Add("price",7.99);
insertDocumentRequest = new InsertDocumentRequest(index: "products", doc: doc);
sqlresult = indexApi.Insert(insertDocumentRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut doc1 = HashMap::new();
doc1.insert("title".to_string(), serde_json::json!("Crossbody Bag with Tassel"));
doc1.insert("price".to_string(), serde_json::json!(19.85));
let insert_req1 = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc1));
let insert_res1 = index_api.insert(insert_req1).await;

let mut doc2 = HashMap::new();
doc2.insert("title".to_string(), serde_json::json!("microfiber sheet set"));
doc2.insert("price".to_string(), serde_json::json!(19.99));
let insert_req2 = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc2));
let insert_res2 = index_api.insert(insert_req2).await;

let mut doc3 = HashMap::new();
doc3.insert("title".to_string(), serde_json::json!("Pet Hair Remover Glove"));
doc3.insert("price".to_string(), serde_json::json!(7.99));
let insert_req3 = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc3));
let insert_res3 = index_api.insert(insert_req3).await;
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

``` typescript
res = await indexApi.insert({
  index: 'test',
  id: 1,
  doc: { content: 'Text 1', name: 'Doc 1', cat: 1 },
});
res = await indexApi.insert({
  index: 'test',
  id: 2,
  doc: { content: 'Text 2', name: 'Doc 2', cat: 2 },
});
res = await indexApi.insert({
  index: 'test',
  id: 3,
  doc: { content: 'Text 3', name: 'Doc 3', cat: 7 },
});
```

<!-- intro -->
##### जाएं:

<!-- request Go -->

``` go
indexDoc := map[string]interface{} {"content": "Text 1", "name": "Doc 1", "cat": 1 }
indexReq := manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(1)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()

indexDoc = map[string]interface{} {"content": "Text 2", "name": "Doc 3", "cat": 2 }
indexReq = manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(2)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()

indexDoc = map[string]interface{} {"content": "Text 3", "name": "Doc 3", "cat": 7 }
indexReq = manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(3)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()
```

<!-- end -->

इस विषय पर अधिक जानकारी यहाँ मिल सकती है:
* [सादा तालिका में डेटा जोड़ना](https://play.manticoresearch.com/mysql/)
* [रीयलटाइम तालिका में डेटा जोड़ना](https://play.manticoresearch.com/rtintro/)


<!-- example search -->
## खोज

चलो हम दस्तावेज़ों में से एक को खोजते हैं। हम जो क्वेरी का उपयोग करेंगे वह है 'बाल हटाएँ'। जैसा कि आप देख सकते हैं, यह 'पेट हेयर रिमूवर ग्लव' शीर्षक वाला एक दस्तावेज़ ढूंढता है और इसमें 'हेयर रिमूवर' को हाइलाइट करता है, भले ही क्वेरी में "हटाएँ", "रिमूवर" न हो। यह इसलिए है क्योंकि जब हमने तालिका बनाई, तो हमने अंग्रेजी स्टेमिंग का उपयोग करने को सक्षम किया (`morphology "stem_en"`).

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
select id, highlight(), price from products where match('remove hair');
```
<!-- response SQL -->

```sql
+---------------------+-------------------------------+----------+
| id                  | highlight()                   | price    |
+---------------------+-------------------------------+----------+
| 1513686608316989452 | Pet <b>Hair Remover</b> Glove | 7.990000 |
+---------------------+-------------------------------+----------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "products",
  "query": { "match": { "title": "remove hair" } },
  "highlight":
  {
    "fields": ["title"]
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "_id": 1513686608316989452,
        "_score": 1680,
        "_source": {
          "price": 7.99,
          "title": "Pet Hair Remover Glove"
        },
        "highlight": {
          "title": [
            "Pet <b>Hair Remover</b> Glove"
          ]
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
$result = $index->search('@title remove hair')->highlight(['title'])->get();
foreach($result as $doc)
{
    echo "Doc ID: ".$doc->getId()."
";
    echo "Doc Score: ".$doc->getScore()."
";
    echo "Document fields:
";
    print_r($doc->getData());
    echo "Highlights: 
";
    print_r($doc->getHighlight());
}
```
<!-- response PHP -->

```php
Doc ID: 1513686608316989452
Doc Score: 1680
Document fields:
Array
(
    [price] => 7.99
    [title] => Pet Hair Remover Glove
)
Highlights:
Array
(
    [title] => Array
        (
            [0] => Pet <b>Hair Remover</b> Glove
        )
)

```
`

<!-- intro -->
पायथन
<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"query_string":"@title remove hair"},"highlight":{"fields":["title"]}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'1513686608316989452',
                    u'_score': 1680,
                    u'_source': {u'title': u'Pet Hair Remover Glove', u'price':7.99},
                    u'highlight':{u'title':[u'Pet <b>Hair Remover</b> Glove']}}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
पायथन-ऐसिंक्रोनस
<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"products","query":{"query_string":"@title remove hair"},"highlight":{"fields":["title"]}})
```
<!-- response Python-asyncio -->
``` python
{'hits': {'hits': [{u'_id': u'1513686608316989452',
                    u'_score': 1680,
                    u'_source': {u'title': u'Pet Hair Remover Glove', u'price':7.99},
                    u'highlight':{u'title':[u'Pet <b>Hair Remover</b> Glove']}}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
जावास्क्रिप्ट
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"query_string":"@title remove hair"}"highlight":{"fields":["title"]}});
```
<!-- response javascript -->
```javascript
{"hits": {"hits": [{"_id": 1513686608316989452,
                    "_score": 1680,
                    "_source": {"title": "Pet Hair Remover Glove", "price":7.99},
                    "highlight":{"title":["Pet <b>Hair Remover</b> Glove"]}}],
          "total": 1},
 "profile": None,
 "timed_out": False,
 "took": 0}
```

<!-- intro -->
जावा
<!-- request Java -->

```java

query = new HashMap<String,Object>();
query.put("query_string","@title remove hair");
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
searchRequest.setQuery(query);
HashMap<String,Object> highlight = new HashMap<String,Object>(){{
    put("fields",new String[] {"title"});

}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
```java
class SearchResponse {
    took: 84
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: null
        hits: [{_id=1513686608316989452, _score=1, _source={price=7.99, title=Pet Hair Remover Glove}, highlight={title=[Pet <b>Hair Remover</b> Glove]}}]
        aggregations: null
    }
    profile: null
}


```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { query_string="@title remove hair" };
var searchRequest = new SearchRequest("products", query);
var highlight = new Highlight();
highlight.Fieldnames = new List<string> {"title"};
searchRequest.Highlight = highlight;
searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
```clike
class SearchResponse {
    took: 103
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: null
        hits: [{_id=1513686608316989452, _score=1, _source={price=7.99, title=Pet Hair Remover Glove}, highlight={title=[Pet <b>Hair Remover</b> Glove]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- intro -->
Rust
<!-- request Rust -->

```rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("@title remove hair").into()),
    ..Default::default()
};
let highlight = Highlight {
    fields: Some(serde_json::json!(["title"]).into()),
    ..Default::default()
};
let search_req = SearchRequest {
    table: "products".to_string(),
    query: Some(Box::new(query)),
    highlight: Some(Box::new(highlight)),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
```rust
class SearchResponse {
    took: 103
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: null
        hits: [{_id=1513686608316989452, _score=1, _source={price=7.99, title=Pet Hair Remover Glove}, highlight={title=[Pet <b>Hair Remover</b> Glove]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({
  index: 'test',
  query: { query_string: {'text 1'} },
  highlight: {'fields': ['content'] }
});
```
<!-- response typescript -->
```typescript
{
    "hits":
    {
        "hits":
        [{
            "_id": 1,
            "_score": 1400,
            "_source": {"content":"Text 1","name":"Doc 1","cat":1},
            "highlight": {"content":["<b>Text 1</b>"]}
        }],
        "total": 1
    },
    "profile": None,
    "timed_out": False,
    "took": 0
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
searchRequest := manticoreclient.NewSearchRequest("test")
query := map[string]interface{} {"query_string": "text 1"};
searchRequest.SetQuery(query);

highlightField := manticoreclient.NewHighlightField("content")
fields := []interface{}{ highlightField }
highlight := manticoreclient.NewHighlight()
highlight.SetFields(fields)
searchRequest.SetHighlight(highlight);

res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
```go
{
    "hits":
    {
        "hits":
        [{
            "_id": 1,
            "_score": 1400,
            "_source": {"content":"Text 1","name":"Doc 1","cat":1},
            "highlight": {"content":["<b>Text 1</b>"]}
        }],
        "total": 1
    },
    "profile": None,
    "timed_out": False,
    "took": 0
}
```
<!-- end -->

Manticore में उपलब्ध विभिन्न खोज विकल्पों पर अधिक जानकारी हमारे शिक्षण पाठ्यक्रमों में मिल सकती है:
* [Faceted search](https://play.manticoresearch.com/faceting/)
* [Geo search](https://play.manticoresearch.com/geosearch/)
* [Searching for similar documents](https://play.manticoresearch.com/mlt/)

<!-- example update -->
## अपडेट

मान लीजिए कि हम अब दस्तावेज़ को अपडेट करना चाहते हैं - कीमत को 18.5 में बदलें। यह किसी भी क्षेत्र द्वारा फ़िल्टर करके किया जा सकता है, लेकिन सामान्यत: आप दस्तावेज़ आईडी जानते हैं और इसके आधार पर कुछ अपडेट करते हैं।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
update products set price=18.5 where id = 1513686608316989452;
```
<!-- response SQL -->

```sql
Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /update
{
  "table": "products",
  "id": 1513686608316989452,
  "doc":
  {
    "price": 18.5
  }
}
```

<!-- response JSON -->

```json
{
  "table": "products",
  "_id": 1513686608316989452,
  "result": "updated"
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$doc = [
    'body' => [
        'table' => 'products',
        'id' => 2,
        'doc' => [
            'price' => 18.5
        ]
    ]
];

$response = $client->update($doc);
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.update({"table" : "products", "id" : 1513686608316989452, "doc" : {"price":18.5}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
await indexApi.update({"table" : "products", "id" : 1513686608316989452, "doc" : {"price":18.5}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"table" : "products", "id" : 1513686608316989452, "doc" : {"price":18.5}});
```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
UpdateDocumentRequest अपडेटRequest = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("price",18.5);
}};
अपडेटRequest.index("products").id(1513686608316989452L).setDoc(doc);
indexApi.update(अपडेटRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("price", 18.5);
UpdateDocumentRequest अपडेटDocumentRequest = new UpdateDocumentRequest(index: "products", id: 1513686608316989452L, doc: doc);
indexApi.Update(अपडेटDocumentRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->
``` rust
let mut doc = HashMap::new();
doc.insert("price".to_string(), serde_json::json!(18.5));
let अपडेट_req = UpdateDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(1513686608316989452),
    ..Default::default(),
};
let अपडेट_res = index_api.update(अपडेट_req).await;
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.update({ index: "test", id: 1, doc: { cat: 10 } });
```

<!-- intro -->
##### Go:

<!-- request Go -->
``` go
updDoc = map[string]interface{} {"cat": 10}
updRequest = manticoreclient.NewUpdateDocumentRequest("test", updDoc)
updRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updRequest).Execute()
```

<!-- end -->

<!-- example delete -->
## Delete

आइए अब सभी दस्तावेज़ों को हटा दें जिनकी कीमत 10 से कम है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
delete from products where price < 10;
```
<!-- response SQL -->

```sql
Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /delete
{
  "table": "products",
  "query":
  {
    "range":
    {
      "price":
      {
        "lte": 10
      }
    }
  }
}
```

<!-- response JSON -->

```json
{
  "table": "products",
  "deleted": 1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$result = $index->deleteDocuments(new ManticoresearchQueryRange('price',['lte'=>10]));

```
<!-- response PHP -->

```php
Array
(
    [_index] => products
    [deleted] => 1
)

```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.delete({"table" : "products", "query": {"range":{"price":{"lte":10}}}})
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
await indexApi.delete({"table" : "products", "query": {"range":{"price":{"lte":10}}}})
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"table" : "products", "query": {"range":{"price":{"lte":10}}}});
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
query = new HashMap<String,Object>();
query.put("range",new HashMap<String,Object>(){{
    put("price",new HashMap<String,Object>(){{
        put("lte",10);
    }});
}});
deleteRequest.index("products").setQuery(query);
indexApi.delete(deleteRequest);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> price = new Dictionary<string, Object>();
price.Add("lte", 10);
Dictionary<string, Object> range = new Dictionary<string, Object>();
range.Add("price", price);
DeleteDocumentRequest deleteDocumentRequest = new DeleteDocumentRequest(index: "products", query: range);
indexApi.Delete(deleteDocumentRequest);
```

<!-- intro -->

##### Rust:

<!-- request Rust -->
``` rust
let mut price_range= HashMap::new();
price_range.insert("lte".to_string(), serde_json::json!(10));
let mut range= HashMap::new();
range.insert("price".to_string(), serde_json::json!(price_range));
let delete_req = DeleteDocumentRequest {
    table: "products".to_string(),
    query: serde_json::json!(range),
    ..Default::default(),
};
index_api.delete(delete_req).await;
```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({
  index: 'test',
  query: { match: { '*': 'Text 1' } },
});
```

<!-- intro -->

##### Go:

<!-- request Go -->
``` go
delRequest := manticoreclient.NewDeleteDocumentRequest("test")
matchExpr := map[string]interface{} {"*": "Text 1t"}
delQuery := map[string]interface{} {"match": matchExpr }
# त्वरित शुरुआत गाइड

<!-- example install -->
## मैंटिकोर इंस्टॉल और शुरू करें

आप विभिन्न ऑपरेटिंग सिस्टम, जैसे उबंटू, सेंटोस, डेबियन, विंडोज और मैकओएस पर आसानी से मैंटिकोर इंस्टॉल और शुरू कर सकते हैं। इसके अलावा, आप मैंटिकोर को डॉकर कंटेनर के रूप में भी उपयोग कर सकते हैं।

<!-- intro -->
### उबंटू

<!-- request Ubuntu -->
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### डेबियन

<!-- request Debian -->
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### सेंटोस

<!-- request Centos -->
```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### विंडोज

<!-- request Windows -->
* https://manticoresearch.com/install/ से विंडोज अर्काइव डाउनलोड करें
* अर्काइव से सभी फ़ाइलें `C:\Manticore` में निकालें
* मैंटिकोर को सेवा के रूप में इंस्टॉल करने के लिए निम्न कमांड चलाएं:
* ```bash
  C:\Manticore\bin\searchd --install --config C:\Manticore\sphinx.conf.in --servicename Manticore
  ```
* माइक्रोसॉफ्ट मैनेजमेंट कंसोल के सेवाओं स्नैप-इन से मैंटिकोर शुरू करें।

<!-- intro -->
### मैकओएस

<!-- request MacOS -->
```bash
brew install manticoresearch
brew services start manticoresearch
```

<!-- intro -->
### डॉकर
<!-- request Docker -->
```bash
docker pull manticoresearch/manticore
docker run --name manticore -p9306:9306 -p9308:9308 -p9312:9312 -d manticoresearch/manticore
```
अपनी डेटा डायरेक्टरी को बनाए रखने के लिए, [मैंटिकोर डॉकर को उत्पादन में कैसे उपयोग करें](Starting_the_server/Docker.md#Production-use) पढ़ें
<!-- end -->

<!-- example connect -->
## मैंटिकोर से कनेक्ट करें

डिफ़ॉल्ट रूप से मैंटिकोर आपके कनेक्शन की प्रतीक्षा कर रहा है:

  * MySQL क्लाइंट्स के लिए पोर्ट 9306
  * HTTP/HTTPS कनेक्शन के लिए पोर्ट 9308
  * अन्य मैंटिकोर नोड्स और मैंटिकोर बाइनरी API पर आधारित क्लाइंट्स के लिए पोर्ट 9312

HTTPS समर्थन के बारे में अधिक विवरण हमारे सीखने के पाठ्यक्रम में [यहां](https://play.manticoresearch.com/https/) मिल सकता है।

<!-- intro -->
##### MySQL के माध्यम से कनेक्ट करें:

<!-- request SQL -->
```bash
mysql -h0 -P9306
```

<!-- intro -->
##### HTTP पर JSON के माध्यम से कनेक्ट करें

<!-- request HTTP -->
HTTP एक बिना स्थिति वाला प्रोटोकॉल है, इसलिए इसे किसी विशेष कनेक्शन चरण की आवश्यकता नहीं होती। आप बस सर्वर को एक HTTP अनुरोध भेज सकते हैं और प्रतिक्रिया प्राप्त कर सकते हैं। JSON इंटरफ़ेस का उपयोग करके मैंटिकोर से संवाद करने के लिए, आप अपनी प्रोग्रामिंग भाषा में किसी भी HTTP क्लाइंट लाइब्रेरी का उपयोग कर सकते हैं जो सर्वर को GET या POST अनुरोध भेजने और JSON प्रतिक्रियाओं को पार्स करने के लिए:

```bash
curl -s "http://localhost:9308/search"
```

<!-- intro -->
##### [PHP क्लाइंट](https://github.com/manticoresoftware/manticoresearch-php) के माध्यम से कनेक्ट करें:

<!-- request PHP -->
```php
// https://github.com/manticoresoftware/manticoresearch-php
require_once __DIR__ . '/vendor/autoload.php';
$config = ['host'=>'127.0.0.1','port'=>9308];
$client = new \Manticoresearch\Client($config);
```

<!-- intro -->
##### [पायथन क्लाइंट](https://github.com/manticoresoftware/manticoresearch-php) के माध्यम से कनेक्ट करें:

<!-- request Python -->
```python
// https://github.com/manticoresoftware/manticoresearch-python
import manticoresearch
config = manticoresearch.Configuration(
    host = "http://127.0.0.1:9308"
)
client = manticoresearch.ApiClient(config)
indexApi = manticoresearch.IndexApi(client)
searchApi = manticoresearch.SearchApi(client)
utilsApi = manticoresearch.UtilsApi(client)
```

<!-- intro -->
##### [जावास्क्रिप्ट क्लाइंट](https://github.com/manticoresoftware/manticoresearch-javascript) के माध्यम से कनेक्ट करें:

<!-- request Javascript -->
```javascript
// https://github.com/manticoresoftware/manticoresearch-javascript
var Manticoresearch = require('manticoresearch');
var client= new Manticoresearch.ApiClient()
client.basePath="http://127.0.0.1:9308";
indexApi = new Manticoresearch.IndexApi(client);
searchApi = new Manticoresearch.SearchApi(client);
utilsApi = new Manticoresearch.UtilsApi(client);
```
<!-- intro -->
##### [जावा क्लाइंट](https://github.com/manticoresoftware/manticoresearch-java) के माध्यम से कनेक्ट करें:

<!-- request Java -->
```java
// https://github.com/manticoresoftware/manticoresearch-java
import com.manticoresearch.client.*;
import com.manticoresearch.client.model.*;
import com.manticoresearch.client.api.*;
...
ApiClient client = Configuration.getDefaultApiClient();
client.setBasePath("http://127.0.0.1:9308");
...
IndexApi indexApi = new IndexApi(client);
SearchApi searchApi = new UtilsApi(client);
UtilsApi utilsApi = new UtilsApi(client);
```

<!-- intro -->
##### [.Net क्लाइंट](https://github.com/manticoresoftware/manticoresearch-net) के माध्यम से कनेक्ट करें:

<!-- request C# -->
```clike
// https://github.com/manticoresoftware/manticoresearch-net
using System.Net.Http;
...
using ManticoreSearch.Client;
using ManticoreSearch.Api;
using ManticoreSearch.Model;
...
config = new Configuration();
config.BasePath = "http://localhost:9308";
httpClient = new HttpClient();
httpClientHandler = new HttpClientHandler();
...
var indexApi = new IndexApi(httpClient, config, httpClientHandler);
var searchApi = new SearchApi(httpClient, config, httpClientHandler);
var utilsApi = new UtilsApi(httpClient, config, httpClientHandler);
```

<!-- intro -->
##### [TypeScript क्लाइंट](https://github.com/manticoresoftware/manticoresearch-typescript) के माध्यम से कनेक्ट करें:

<!-- request Typescript -->
```typescript
import {
  Configuration,
  IndexApi,
  SearchApi,
  UtilsApi
} from "manticoresearch-ts";
...
const config = new Configuration({
  basePath: 'http://localhost:9308',
})
const indexApi = new IndexApi(config);
const searchApi = new SearchApi(config);
const utilsApi = new UtilsApi(config);
```

<!-- intro -->
##### [Go क्लाइंट](https://github.com/manticoresoftware/manticoresearch-go) के माध्यम से कनेक्ट करें:

<!-- request Go -->
```go
import (
	"context"
	manticoreclient "github.com/manticoresoftware/manticoresearch-go"
)
...
configuration := manticoreclient.NewConfiguration()
configuration.Servers[0].URL = "http://localhost:9308"
apiClient := manticoreclient.NewAPIClient(configuration)
```

<!-- end -->

<!-- example create -->
## एक तालिका बनाएँ

अब हम "products" नाम की एक तालिका बनाएँगे जिसमें 2 क्षेत्र हैं:
* title - पूर्ण-ग्रंथ क्षेत्र जिसमें हमारे उत्पाद का शीर्षक होगा
* price - प्रकार "float" का

ध्यान दें कि स्पष्ट निर्माण कथन के साथ तालिका बनाना छोड़ना संभव है। अधिक जानकारी के लिए देखें [ऑटो स्कीमा](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

एक तालिका बनाने के विभिन्न तरीकों के बारे में अधिक जानकारी हमारे शिक्षण पाठ्यक्रमों में मिल सकती है:
* [एक रियलटाइम तालिका बनाना](https://play.manticoresearch.com/rtmode/)
* [MySQL स्रोत से तालिका बनाना](https://play.manticoresearch.com/mysql/)
* [CSV स्रोत से तालिका बनाना](https://play.manticoresearch.com/csv/)
* [ऑटो स्कीमा तंत्र का उपयोग करके तालिका बनाना](https://play.manticoresearch.com/autoschema/)
* [Logstash/Beats के साथ तालिका बनाना](https://play.manticoresearch.com/logstash/)
* [Fluentbit के साथ तालिका बनाना](https://play.manticoresearch.com/vectordev/)
* [Vector.dev एजेंट का उपयोग करके तालिका बनाना](https://play.manticoresearch.com/vectordev/)

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
create table products(title text, price float) morphology='stem_en';
```
<!-- response -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```bash
POST /cli -d "create table products(title text, price float) morphology='stem_en'"
```

<!-- response HTTP -->
```json
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
    'price'=>['type'=>'float'],
],['morphology' => 'stem_en']);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('create table products(title text, price float) morphology=\'stem_en\'')
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('create table products(title text, price float) morphology=\'stem_en\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("create table products(title text, price float) morphology='stem_en'");

```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("create table products(title text, price float) morphology='stem_en'");

```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
```typescript
res = await utilsApi.sql('create table products(title text, price float) morphology=\'stem_en\'');
```

<!-- intro -->
##### Go:

<!-- request Go -->
```go
res := apiClient.UtilsAPI.Sql(context.Background()).Body("create table products(title text, price float) morphology='stem_en'").Execute();
```

<!-- end -->

<!-- example insert -->
## दस्तावेज़ जोड़ें

अब हम तालिका में कुछ दस्तावेज़ जोड़ेंगे:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
insert into products(title,price) values ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->
`"id":0` या कोई आईडी स्वचालित आईडी उत्पादन को मजबूर करती है।

```json
POST /insert
{
  "table":"products",
  "doc":
  {
    "title" : "Crossbody Bag with Tassel",
    "price" : 19.85
  }
}


POST /insert
{
  "table":"products",
  "doc":
  {
    "title" : "microfiber sheet set",
    "price" : 19.99
  }
}

POST /insert
{
  "table":"products",
  "doc":
  {
    "title" : "Pet Hair Remover Glove",
    "price" : 7.99
  }
}
```
<!-- response JSON -->

```json
{
  "table": "products",
  "_id": 0,
  "created": true,
  "result": "created",
  "status": 201
}

{
  "table": "products",
  "_id": 0,
  "created": true,
  "result": "created",
  "status": 201
}

{
  "table": "products",
  "_id": 0,
  "created": true,
  "result": "created",
  "status": 201
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->addDocuments([
        ['title' => 'Crossbody Bag with Tassel', 'price' => 19.85],
        ['title' => 'microfiber sheet set', 'price' => 19.99],
        ['title' => 'Pet Hair Remover Glove', 'price' => 7.99]
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

``` python
indexApi.insert({"table" : "products", "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}})
indexApi.insert({"table" : "products", "doc" : {"title" : "microfiber sheet set", "price" : 19.99}})
indexApi.insert({"table" : "products", "doc" : {"title" : "पेट हेयर रिमूवर ग्लव", "price" : 7.99}})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "क्रॉसबॉडी बैग विद टैसल", "price" : 19.85}});
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "माइक्रोफाइबर शीट सेट", "price" : 19.99}});
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "पेट हेयर रिमूवर ग्लव", "price" : 7.99}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","क्रॉसबॉडी बैग विद टैसल");
    put("price",19.85);
}};
newdoc.index("products").setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
doc = new HashMap<String,Object>(){{
    put("title","माइक्रोफाइबर शीट सेट");
    put("price",19.99);
}};
newdoc.index("products").setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
doc = new HashMap<String,Object>(){{
    put("title","पेट हेयर रिमूवर ग्लव");
    put("price",7.99);
 }};
newdoc.index("products").setDoc(doc);
indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title","क्रॉसबॉडी बैग विद टैसल");
doc.Add("price",19.85);
InsertDocumentRequest insertDocumentRequest = new InsertDocumentRequest(index: "products", doc: doc);
sqlresult = indexApi.Insert(insertDocumentRequest);

doc = new Dictionary<string, Object>();
doc.Add("title","माइक्रोफाइबर शीट सेट");
doc.Add("price",19.99);
insertDocumentRequest = new InsertDocumentRequest(index: "products", doc: doc);
sqlresult = indexApi.Insert(insertDocumentRequest);

doc = new Dictionary<string, Object>();
doc.Add("title","पेट हेयर रिमूवर ग्लव");
doc.Add("price",7.99);
insertDocumentRequest = new InsertDocumentRequest(index: "products", doc: doc);
sqlresult = indexApi.Insert(insertDocumentRequest);
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

``` typescript
res = await indexApi.insert({
  index: 'test',
  id: 1,
  doc: { content: 'टेक्स्ट 1', name: 'डॉक 1', cat: 1 },
});
res = await indexApi.insert({
  index: 'test',
  id: 2,
  doc: { content: 'टेक्स्ट 2', name: 'डॉक 2', cat: 2 },
});
res = await indexApi.insert({
  index: 'test',
  id: 3,
  doc: { content: 'टेक्स्ट 3', name: 'डॉक 3', cat: 7 },
});
```

<!-- intro -->
##### Go:

<!-- request Go -->

``` go
indexDoc := map[string]interface{} {"content": "टेक्स्ट 1", "name": "डॉक 1", "cat": 1 }
indexReq := manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(1)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()

indexDoc = map[string]interface{} {"content": "टेक्स्ट 2", "name": "डॉक 3", "cat": 2 }
indexReq = manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(2)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()

indexDoc = map[string]interface{} {"content": "टेक्स्ट 3", "name": "डॉक 3", "cat": 7 }
indexReq = manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(3)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()
```

<!-- end -->

More details on the subject can be found here:
* [Adding data to a plain table](https://play.manticoresearch.com/mysql/)
* [Adding data to a RealTime table](https://play.manticoresearch.com/rtintro/)


<!-- example search -->
## Search

Let's find one of the documents. The query we will use is 'remove hair'. As you can see, it finds a document with the title 'पेट हेयर रिमूवर ग्लव' and highlights 'Hair remover' in it, even though the query has "remove", not "remover". This is because when we created the table, we turned on using English stemming (`morphology "stem_en"`).

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
select id, highlight(), price from products where match('remove hair');
```
<!-- response SQL -->

```sql
+---------------------+-------------------------------+----------+
| id                  | highlight()                   | price    |
+---------------------+-------------------------------+----------+
| 1513686608316989452 | पेट <b>हेयर रिमूवर</b> ग्लव | 7.990000 |
+---------------------+-------------------------------+----------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "products",
  "query": { "match": { "title": "remove hair" } },
  "highlight":
  {
    "fields": ["title"]
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "_id": 1513686608316989452,
        "_score": 1680,
        "_source": {
          "price": 7.99,
          "title": "पेट हेयर रिमूवर ग्लव"
        },
        "highlight": {
          "title": [
            "पेट <b>हेयर रिमूवर</b> ग्लव"
          ]
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
$result = $index->search('@title remove hair')->highlight(['title'])->get();
foreach($result as $doc)
{
    echo "Doc ID: ".$doc->getId()."\n";
    echo "Doc Score: ".$doc->getScore()."\n";
    echo "Document fields:\n";
    print_r($doc->getData());
    echo "Highlights: \n";
    print_r($doc->getHighlight());
}
```
<!-- response PHP -->

```php
Doc ID: 1513686608316989452
Doc Score: 1680
Document fields:
Array
(
    [price] => 7.99
    [title] => पालतू बाल हटाने वाला दस्ताना
)
Highlights:
Array
(
    [title] => Array
        (
            [0] => पालतू <b>बाल हटाने वाला</b> दस्ताना
        )
)

```
`

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"products","query":{"query_string":"@title remove hair"},"highlight":{"fields":["title"]}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'1513686608316989452',
                    u'_score': 1680,
                    u'_source': {u'title': u'पालतू बाल हटाने वाला दस्ताना', u'price':7.99},
                    u'highlight':{u'title':[u'पालतू <b>बाल हटाने वाला</b> दस्ताना']}}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"products","query":{"query_string":"@title remove hair"}"highlight":{"fields":["title"]}});
```
<!-- response javascript -->
```javascript
{"hits": {"hits": [{"_id": 1513686608316989452,
                    "_score": 1680,
                    "_source": {"title": "पालतू बाल हटाने वाला दस्ताना", "price":7.99},
                    "highlight":{"title":["पालतू <b>बाल हटाने वाला</b> दस्ताना"]}}],
          "total": 1},
 "profile": None,
 "timed_out": False,
 "took": 0}
```

<!-- intro -->
java
<!-- request Java -->

```java

query = new HashMap<String,Object>();
query.put("query_string","@title remove hair");
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
searchRequest.setQuery(query);
HashMap<String,Object> highlight = new HashMap<String,Object>(){{
    put("fields",new String[] {"title"});

}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
```java
class SearchResponse {
    took: 84
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: null
        hits: [{_id=1513686608316989452, _score=1, _source={price=7.99, title=पालतू बाल हटाने वाला दस्ताना}, highlight={title=[पालतू <b>बाल हटाने वाला</b> दस्ताना]}}]
        aggregations: null
    }
    profile: null
}


```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { query_string="@title remove hair" };
var searchRequest = new SearchRequest("products", query);
var highlight = new Highlight();
highlight.Fieldnames = new List<string> {"title"};
searchRequest.Highlight = highlight;
searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
```clike
class SearchResponse {
    took: 103
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: null
        hits: [{_id=1513686608316989452, _score=1, _source={price=7.99, title=पालतू बाल हटाने वाला दस्ताना}, highlight={title=[पालतू <b>बाल हटाने वाला</b> दस्ताना]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({
  index: 'test',
  query: { query_string: {'text 1'} },
  highlight: {'fields': ['content'] }
});
```
<!-- response typescript -->
```typescript
{
    "hits":
    {
        "hits":
        [{
            "_id": 1,
            "_score": 1400,
            "_source": {"content":"पाठ 1","name":"Doc 1","cat":1},
            "highlight": {"content":["<b>पाठ 1</b>"]}
        }],
        "total": 1
    },
    "profile": None,
    "timed_out": False,
    "took": 0
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
searchRequest := manticoreclient.NewSearchRequest("test")
query := map[string]interface{} {"query_string": "text 1"};
searchRequest.SetQuery(query);

highlightField := manticoreclient.NewHighlightField("content")
fields := []interface{}{ highlightField }
highlight := manticoreclient.NewHighlight()
highlight.SetFields(fields)
searchRequest.SetHighlight(highlight);

res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
```go
{
    "hits":
    {
        "hits":
        [{
            "_id": 1,
            "_score": 1400,
            "_source": {"content":"पाठ 1","name":"Doc 1","cat":1},
            "highlight": {"content":["<b>पाठ 1</b>"]}
        }],
        "total": 1
    },
    "profile": None,
    "timed_out": False,
    "took": 0
}
```
<!-- end -->

More information on different search options available in Manticore can be found in our learning courses:
* [Faceted search](https://play.manticoresearch.com/faceting/)
* [Geo search](https://play.manticoresearch.com/geosearch/)
* [Searching for similar documents](https://play.manticoresearch.com/mlt/)

<!-- example update -->
## Update

Let's assume we now want to update the document - change the price to 18.5. This can be done by filtering by any field, but normally you know the document id and update something based on that.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
update products set price=18.5 where id = 1513686608316989452;
```
<!-- response SQL -->

```sql
Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /update
{
  "table": "products",
  "id": 1513686608316989452,
  "doc":
  {
    "price": 18.5
  }
}
```

<!-- response JSON -->

```json
{
  "table": "products",
  "_id": 1513686608316989452,
  "result": "updated"
}
```
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$doc = [
    'body' => [
        'table' => 'products',
        'id' => 2,
        'doc' => [
            'price' => 18.5
        ]
    ]
];

$response = $client->update($doc);
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.update({"table" : "products", "id" : 1513686608316989452, "doc" : {"price":18.5}})
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"table" : "products", "id" : 1513686608316989452, "doc" : {"price":18.5}});
```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("price",18.5);
}};
updateRequest.index("products").id(1513686608316989452L).setDoc(doc);
indexApi.update(updateRequest);
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("price", 18.5);
UpdateDocumentRequest updateDocumentRequest = new UpdateDocumentRequest(index: "products", id: 1513686608316989452L, doc: doc);
indexApi.Update(updateDocumentRequest);
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.update({ index: "test", id: 1, doc: { cat: 10 } });
```

<!-- intro -->
##### Go:

<!-- request Go -->
``` go
updDoc = map[string]interface{} {"cat": 10}
updRequest = manticoreclient.NewUpdateDocumentRequest("test", updDoc)
updRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updRequest).Execute()
```

<!-- end -->

<!-- example delete -->
## Delete

चलो अब उन सभी दस्तावेज़ों को हटा देते हैं जिनकी कीमत 10 से कम है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
delete from products where price < 10;
```
<!-- response SQL -->

```sql
Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /delete
{
  "table": "products",
  "query":
  {
    "range":
    {
      "price":
      {
        "lte": 10
      }
    }
  }
}
```

<!-- response JSON -->

```json
{
  "table": "products",
  "deleted": 1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$result = $index->deleteDocuments(new \Manticoresearch\Query\Range('price',['lte'=>10]));

```
<!-- response PHP -->

```php
Array
(
    [_index] => products
    [deleted] => 1
)

```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.delete({"table" : "products", "query": {"range":{"price":{"lte":10}}}})
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"table" : "products", "query": {"range":{"price":{"lte":10}}}});
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
query = new HashMap<String,Object>();
query.put("range",new HashMap<String,Object>(){{
    put("price",new HashMap<String,Object>(){{
        put("lte",10);
    }});
}});
deleteRequest.index("products").setQuery(query);
indexApi.delete(deleteRequest);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> price = new Dictionary<string, Object>();
price.Add("lte", 10);
Dictionary<string, Object> range = new Dictionary<string, Object>();
range.Add("price", price);
DeleteDocumentRequest deleteDocumentRequest = new DeleteDocumentRequest(index: "products", range: range);
indexApi.Delete(deleteDocumentRequest);
```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({
  index: 'test',
  query: { match: { '*': 'Text 1' } },
});
```

<!-- intro -->

##### Go:

<!-- request Go -->
``` go
delRequest := manticoreclient.NewDeleteDocumentRequest("test")
matchExpr := map[string]interface{} {"*": "Text 1t"}
delQuery := map[string]interface{} {"match": matchExpr }
delRequest.SetQuery(delQuery)
res, _, _ := apiClient.IndexAPI.Delete(context.Background()).DeleteDocumentRequest(*delRequest).Execute();
```

<!-- end -->
<!-- proofread -->

