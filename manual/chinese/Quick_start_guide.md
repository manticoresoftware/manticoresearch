# 快速入门指南

<!-- example install -->
## 安装并启动 Manticore

您可以在各种操作系统上轻松安装并启动 Manticore，包括 Ubuntu、Centos、Debian、Windows 和 MacOS。此外，您还可以将 Manticore 用作 Docker 容器。

<!-- intro -->
### Ubuntu

<!-- request Ubuntu -->
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### Debian

<!-- request Debian -->
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### Centos

<!-- request Centos -->
```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### Windows

<!-- request Windows -->
* 从 https://manticoresearch.com/install/ 下载 Windows 压缩包
* 将压缩包中的所有文件解压到 `C:Manticore`
* 运行以下命令将 Manticore 安装为服务：
* ```bash
  C:Manticoreinsearchd --install --config C:Manticoresphinx.conf.in --servicename Manticore
  ```
* 从 Microsoft 管理控制台的服务管理单元中启动 Manticore。

<!-- intro -->
### MacOS

<!-- request MacOS -->
```bash
brew install manticoresearch
brew services start manticoresearch
```

<!-- intro -->
### Docker
<!-- request Docker -->
```bash
docker pull manticoresearch/manticore
docker run --name manticore -p9306:9306 -p9308:9308 -p9312:9312 -d manticoresearch/manticore
```
要持久化数据目录，请阅读 [如何在生产中使用 Manticore Docker](Starting_the_server/Docker.md#Production-use)
<!-- end -->

<!-- example connect -->
## 连接到 Manticore

默认情况下 Manticore 正在等待您的连接：

  * 9306 端口用于 MySQL 客户端
  * 9308 端口用于 HTTP/HTTPS 连接
  * 9312 端口用于来自其他 Manticore 节点和基于 Manticore 二进制 API 的客户端的连接

关于 HTTPS 支持的更多细节可以在我们的学习课程中找到 [这里](https://play.manticoresearch.com/https/)。

<!-- intro -->
##### 通过 MySQL 连接：

<!-- request SQL -->
```bash
mysql -h0 -P9306
```

<!-- intro -->
##### 通过 JSON 进行 HTTP 连接

<!-- request HTTP -->
HTTP 是无状态协议，因此不需要任何特殊的连接阶段。您只需向服务器发送 HTTP 请求并接收响应。要使用 JSON 接口与 Manticore 通信，您可以使用您选择的编程语言中的任何 HTTP 客户端库向服务器发送 GET 或 POST 请求并解析 JSON 响应：

```bash
curl -s "http://localhost:9308/search"
```

<!-- intro -->
##### 通过 [PHP 客户端](https://github.com/manticoresoftware/manticoresearch-php) 连接：

<!-- request PHP -->
```php
// https://github.com/manticoresoftware/manticoresearch-php
require_once __DIR__ . '/vendor/autoload.php';
$config = ['host'=>'127.0.0.1','port'=>9308];
$client = new ManticoresearchClient($config);
```

<!-- intro -->
##### 通过 [Python 客户端](https://github.com/manticoresoftware/manticoresearch-php) 连接：

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
##### 通过 [Python asyncio 客户端](https://github.com/manticoresoftware/manticoresearch-php-asyncio) 连接：

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
##### 通过 [Javascript 客户端](https://github.com/manticoresoftware/manticoresearch-javascript) 连接：

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
##### 通过 [Java 客户端](https://github.com/manticoresoftware/manticoresearch-java) 连接：

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
##### 通过 [.Net 客户端](https://github.com/manticoresoftware/manticoresearch-net) 连接：

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
##### 通过 [Rust 客户端](https://github.com/manticoresoftware/manticoresearch-rust):

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
##### 通过 [TypeScript 客户端](https://github.com/manticoresoftware/manticoresearch-typescript):

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
##### 通过 [Go 客户端](https://github.com/manticoresoftware/manticoresearch-go):

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
## 创建一个表

现在让我们创建一个名为 "products" 的表，它有 2 个字段:
* title - 全文字段，用于存储产品标题
* price - 类型为 "float"

请注意，可以省略使用显式创建语句来创建表。更多信息请参见 [自动模式](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

有关创建表的不同方法的更多信息，请参阅我们的学习课程：
* [创建一个实时表](https://play.manticoresearch.com/rtmode/)
* [从 MySQL 源创建表](https://play.manticoresearch.com/mysql/)
* [从 CSV 源创建表](https://play.manticoresearch.com/csv/)
* [使用自动模式机制创建表](https://play.manticoresearch.com/autoschema/)
* [使用 Logstash/Beats 创建表](https://play.manticoresearch.com/logstash/)
* [使用 Fluentbit 创建表](https://play.manticoresearch.com/vectordev/)
* [使用 Vector.dev agent 创建表](https://play.manticoresearch.com/vectordev/)

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
## 添加文档

现在让我们向该表添加几个文档：

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
插入到产品(title, price) 值 ('流苏单肩包', 19.85), ('超细纤维床单套', 19.99), ('宠物去毛手套', 7.99);
```
<!-- response SQL -->

```sql
查询成功, 3 行受影响 (0.01 秒)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->
`"id":0` 或没有 id 强制自动 ID 生成。

```json
POST /insert
{
  "table":"products",
  "doc":
  {
    "title" : "流苏单肩包",
    "price" : 19.85
  }
}


POST /insert
{
  "table":"products",
  "doc":
  {
    "title" : "超细纤维床单套",
    "price" : 19.99
  }
}

POST /insert
{
  "table":"products",
  "doc":
  {
    "title" : "宠物去毛手套",
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
        ['title' => '流苏单肩包', 'price' => 19.85],
        ['title' => '超细纤维床单套', 'price' => 19.99],
        ['title' => '宠物去毛手套', 'price' => 7.99]
]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

``` python
indexApi.insert({"table" : "products", "doc" : {"title" : "流苏单肩包", "price" : 19.85}})
indexApi.insert({"table" : "products", "doc" : {"title" : "超细纤维床单套", "price" : 19.99}})
indexApi.insert({"table" : "products", "doc" : {"title" : "宠物去毛手套", "price" : 7.99}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

``` python
await indexApi.insert({"table" : "products", "doc" : {"title" : "流苏单肩包", "price" : 19.85}})
await indexApi.insert({"table" : "products", "doc" : {"title" : "超细纤维床单套", "price" : 19.99}})
await indexApi.insert({"table" : "products", "doc" : {"title" : "宠物去毛手套", "price" : 7.99}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "流苏单肩包", "price" : 19.85}});
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "超细纤维床单套", "price" : 19.99}});
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "宠物去毛手套", "price" : 7.99}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","流苏单肩包");
    put("price",19.85);
}};
newdoc.index("products").setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
doc = new HashMap<String,Object>(){{
    put("title","超细纤维床单套");
    put("price",19.99);
}};
newdoc.index("products").setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
doc = new HashMap<String,Object>(){{
    put("title","宠物去毛手套");
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
doc.Add("title","流苏单肩包");
doc.Add("price",19.85);
InsertDocumentRequest insertDocumentRequest = new InsertDocumentRequest(index: "products", doc: doc);
sqlresult = indexApi.Insert(insertDocumentRequest);

doc = new Dictionary<string, Object>();
doc.Add("title","超细纤维床单套");
doc.Add("price",19.99);
insertDocumentRequest = new InsertDocumentRequest(index: "products", doc: doc);
sqlresult = indexApi.Insert(insertDocumentRequest);

doc = new Dictionary<string, Object>();
doc.Add("title","宠物去毛手套");
doc.Add("price",7.99);
insertDocumentRequest = new InsertDocumentRequest(index: "products", doc: doc);
sqlresult = indexApi.Insert(insertDocumentRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut doc1 = HashMap::new();
doc1.insert("title".to_string(), serde_json::json!("流苏单肩包"));
doc1.insert("price".to_string(), serde_json::json!(19.85));
let insert_req1 = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc1));
let insert_res1 = index_api.insert(insert_req1).await;

let mut doc2 = HashMap::new();
doc2.insert("title".to_string(), serde_json::json!("超细纤维床单套"));
doc2.insert("price".to_string(), serde_json::json!(19.99));
let insert_req2 = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc2));
let insert_res2 = index_api.insert(insert_req2).await;

let mut doc3 = HashMap::new();
doc3.insert("title".to_string(), serde_json::json!("宠物去毛手套"));
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
  doc: { content: '文本 1', name: '文档 1', cat: 1 },
});
res = await indexApi.insert({
  index: 'test',
  id: 2,
  doc: { content: '文本 2', name: '文档 2', cat: 2 },
});
res = await indexApi.insert({
  index: 'test',
  id: 3,
  doc: { content: '文本 3', name: '文档 3', cat: 7 },
});
```

<!-- intro -->
##### Go:

<!-- request Go -->

``` go
indexDoc := map[string]interface{} {"content": "文本 1", "name": "文档 1", "cat": 1 }
indexReq := manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(1)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()

indexDoc = map[string]interface{} {"content": "文本 2", "name": "文档 3", "cat": 2 }
indexReq = manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(2)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()

indexDoc = map[string]interface{} {"content": "文本 3", "name": "文档 3", "cat": 7 }
indexReq = manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(3)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()
```

<!-- end -->

More details on the subject can be found here:
* [向普通表添加数据](https://play.manticoresearch.com/mysql/)
* [向实时表添加数据](https://play.manticoresearch.com/rtintro/)


<!-- example search -->
## Search

Let's find one of the documents. The query we will use is 'remove hair'. As you can see, it finds a document with the title 'Pet Hair Remover Glove' and highlights 'Hair remover' in it, even though the query has "remove", not "remover". This is because when we created the table, we turned on using English stemming (`morphology "stem_en"`).

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
    echo "文档 ID: ".$doc->getId()."
";
    echo "文档 得分: ".$doc->getScore()."
";
    echo "文档字段:
";
    print_r($doc->getData());
    echo "高亮:
";
    print_r($doc->getHighlight());
}
```
<!-- response PHP -->

```php
Document ID: 1513686608316989452
Document Score: 1680
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
Python
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
Python-asyncio
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
javascript
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
        hits: [{_id=1513686608316989452, _score=1, _source={price=7.99, title=宠物去毛手套}, highlight={title=[宠物 <b>去毛</b> 手套]}}]
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
        hits: [{_id=1513686608316989452, _score=1, _source={price=7.99, title=宠物去毛手套}, highlight={title=[宠物 <b>去毛</b> 手套]}}]
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
        hits: [{_id=1513686608316989452, _score=1, _source={price=7.99, title=宠物去毛手套}, highlight={title=[宠物 <b>去毛</b> 手套]}}]
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
            "_source": {"content":"文本 1","name":"文档 1","cat":1},
            "highlight": {"content":["<b>文本 1</b>"]}
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
            "_source": {"content":"文本 1","name":"文档 1","cat":1},
            "highlight": {"content":["<b>文本 1</b>"]}
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
* [分面搜索](https://play.manticoresearch.com/faceting/)
* [地理搜索](https://play.manticoresearch.com/geosearch/)
* [搜索相似文档](https://play.manticoresearch.com/mlt/)

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
##### Rust:

<!-- request Rust -->
``` rust
let mut doc = HashMap::new();
doc.insert("price".to_string(), serde_json::json!(18.5));
let update_req = UpdateDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(1513686608316989452),
    ..Default::default(),
};
let update_res = index_api.update(update_req).await;
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

Let's now delete all documents with price lower than 10.

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
# 快速入门指南

<!-- example install -->
## 安装并启动 Manticore

您可以在包括 Ubuntu、Centos、Debian、Windows 和 MacOS 在内的各种操作系统上轻松安装并启动 Manticore。此外，您还可以将 Manticore 用作 Docker 容器。

<!-- intro -->
### Ubuntu

<!-- request Ubuntu -->
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### Debian

<!-- request Debian -->
```bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### Centos

<!-- request Centos -->
```bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore manticore-columnar-lib
sudo systemctl start manticore
```

<!-- intro -->
### Windows

<!-- request Windows -->
* 从 https://manticoresearch.com/install/ 下载 Windows 归档文件
* 将归档中的所有文件解压到 `C:\Manticore`
* 运行以下命令，将 Manticore 安装为服务:
* ```bash
  C:\Manticore\bin\searchd --install --config C:\Manticore\sphinx.conf.in --servicename Manticore
  ```
* 从 Microsoft 管理控制台的服务管理模块中启动 Manticore.

<!-- intro -->
### MacOS

<!-- request MacOS -->
```bash
brew install manticoresearch
brew services start manticoresearch
```

<!-- intro -->
### Docker
<!-- request Docker -->
```bash
docker pull manticoresearch/manticore
docker run --name manticore -p9306:9306 -p9308:9308 -p9312:9312 -d manticoresearch/manticore
```
若要持久化您的数据目录，请阅读 [如何在生产环境中使用 Manticore docker](Starting_the_server/Docker.md#Production-use)
<!-- end -->

<!-- example connect -->
## 连接到 Manticore

默认情况下，Manticore 正在等待您的连接于:

  * 9306 端口用于 MySQL 客户端
  * 9308 端口用于 HTTP/HTTPS 连接
  * 9312 端口用于来自其他 Manticore 节点以及基于 Manticore 二进制 API 的客户端的连接

有关 HTTPS 支持的更多详情，请参阅我们的学习课程 [此处](https://play.manticoresearch.com/https/).

<!-- intro -->
##### 通过 MySQL 连接:

<!-- request SQL -->
```bash
mysql -h0 -P9306
```

<!-- intro -->
##### 通过 HTTP 的 JSON 接口连接

<!-- request HTTP -->
HTTP 是一种无状态协议，因此不需要任何特殊的连接阶段。您只需向服务器发送 HTTP 请求并接收响应即可。要使用 JSON 接口与 Manticore 通信，您可以使用您所选编程语言中的任何 HTTP 客户端库发送 GET 或 POST 请求到服务器，并解析 JSON 响应:

```bash
curl -s "http://localhost:9308/search"
```

<!-- intro -->
##### 通过 [PHP 客户端](https://github.com/manticoresoftware/manticoresearch-php) 连接:

<!-- request PHP -->
```php
// https://github.com/manticoresoftware/manticoresearch-php
require_once __DIR__ . '/vendor/autoload.php';
$config = ['host'=>'127.0.0.1','port'=>9308];
$client = new \Manticoresearch\Client($config);
```

<!-- intro -->
##### 通过 [Python 客户端](https://github.com/manticoresoftware/manticoresearch-php) 连接:

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
##### 通过 [Javascript 客户端](https://github.com/manticoresoftware/manticoresearch-javascript) 连接:

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
##### 通过 [Java 客户端](https://github.com/manticoresoftware/manticoresearch-java) 连接:

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
##### 通过 [.Net 客户端](https://github.com/manticoresoftware/manticoresearch-net) 连接:

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
##### 通过 [TypeScript 客户端](https://github.com/manticoresoftware/manticoresearch-typescript)连接：

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
##### 通过 [Go 客户端](https://github.com/manticoresoftware/manticoresearch-go)连接：

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
## 创建表

现在让我们创建一个名为"products"的表，包含2个字段：
* title - 全文字段，将包含我们的产品标题
* price - 类型为"float"

请注意，可以省略显式创建表的语句。更多信息，请参见[自动架构](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

关于创建表的不同方法的更多信息可以在我们的学习课程中找到：
* [创建实时表](https://play.manticoresearch.com/rtmode/)
* [从MySQL源创建表](https://play.manticoresearch.com/mysql/)
* [从CSV源创建表](https://play.manticoresearch.com/csv/)
* [使用自动架构机制创建表](https://play.manticoresearch.com/autoschema/)
* [使用Logstash/Beats创建表](https://play.manticoresearch.com/logstash/)
* [使用Fluentbit创建表](https://play.manticoresearch.com/vectordev/)
* [使用Vector.dev代理创建表](https://play.manticoresearch.com/vectordev/)

<!-- intro -->
##### SQL：
<!-- request SQL -->

```sql
create table products(title text, price float) morphology='stem_en';
```
<!-- response -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### HTTP：

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
##### PHP：

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('create table products(title text, price float) morphology=\'stem_en\'')
```
<!-- intro -->
##### Javascript：

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('create table products(title text, price float) morphology=\'stem_en\'');
```

<!-- intro -->
##### Java：

<!-- request Java -->

```java
utilsApi.sql("create table products(title text, price float) morphology='stem_en'");

```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("create table products(title text, price float) morphology='stem_en'");

```

<!-- intro -->
##### TypeScript：

<!-- request TypeScript -->
```typescript
res = await utilsApi.sql('create table products(title text, price float) morphology=\'stem_en\'');
```

<!-- intro -->
##### Go：

<!-- request Go -->
```go
res := apiClient.UtilsAPI.Sql(context.Background()).Body("create table products(title text, price float) morphology='stem_en'").Execute();
```

<!-- end -->

<!-- example insert -->
## 添加文档

现在让我们向表中添加几个文档：

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
insert into products(title,price) values ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

<!-- intro -->
##### JSON：

<!-- request JSON -->
使用 `"id":0` 或没有id会强制自动生成ID。

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
##### PHP：

<!-- request PHP -->

```php
$index->addDocuments([
        ['title' => 'Crossbody Bag with Tassel', 'price' => 19.85],
        ['title' => 'microfiber sheet set', 'price' => 19.99],
        ['title' => 'Pet Hair Remover Glove', 'price' => 7.99]
]);
```
<!-- intro -->
##### Python：

<!-- request Python -->

``` python
indexApi.insert({"table" : "products", "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}})
indexApi.insert({"table" : "products", "doc" : {"title" : "microfiber sheet set", "price" : 19.99}})
indexApi.insert({"table" : "products", "doc" : {"title" : "宠物毛发去除手套", "price" : 7.99}})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "流苏斜挎包", "price" : 19.85}});
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "超细纤维床单套件", "price" : 19.99}});
res = await indexApi.insert({"table" : "products", "doc" : {"title" : "宠物毛发去除手套", "price" : 7.99}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","流苏斜挎包");
    put("price",19.85);
}};
newdoc.index("products").setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
doc = new HashMap<String,Object>(){{
    put("title","超细纤维床单套件");
    put("price",19.99);
}};
newdoc.index("products").setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
doc = new HashMap<String,Object>(){{
    put("title","宠物毛发去除手套");
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
doc.Add("title","流苏斜挎包");
doc.Add("price",19.85);
InsertDocumentRequest insertDocumentRequest = new InsertDocumentRequest(index: "products", doc: doc);
sqlresult = indexApi.Insert(insertDocumentRequest);

doc = new Dictionary<string, Object>();
doc.Add("title","超细纤维床单套件");
doc.Add("price",19.99);
insertDocumentRequest = new InsertDocumentRequest(index: "products", doc: doc);
sqlresult = indexApi.Insert(insertDocumentRequest);

doc = new Dictionary<string, Object>();
doc.Add("title","宠物毛发去除手套");
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
  doc: { content: '文本 1', name: '文档 1', cat: 1 },
});
res = await indexApi.insert({
  index: 'test',
  id: 2,
  doc: { content: '文本 2', name: '文档 2', cat: 2 },
});
res = await indexApi.insert({
  index: 'test',
  id: 3,
  doc: { content: '文本 3', name: '文档 3', cat: 7 },
});
```

<!-- intro -->
##### Go:

<!-- request Go -->

``` go
indexDoc := map[string]interface{} {"content": "文本 1", "name": "文档 1", "cat": 1 }
indexReq := manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(1)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()

indexDoc = map[string]interface{} {"content": "文本 2", "name": "文档 3", "cat": 2 }
indexReq = manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(2)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()

indexDoc = map[string]interface{} {"content": "文本 3", "name": "文档 3", "cat": 7 }
indexReq = manticoreclient.NewInsertDocumentRequest("products", indexDoc)
indexReq.SetId(3)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute()
```

<!-- end -->

More details on the subject can be found here:
* [将数据添加到普通表中](https://play.manticoresearch.com/mysql/)
* [将数据添加到实时表中](https://play.manticoresearch.com/rtintro/)


<!-- example search -->
## Search

Let's find one of the documents. The query we will use is 'remove hair'. As you can see, it finds a document with the title '宠物毛发去除手套' and highlights 'Hair remover' in it, even though the query has "remove", not "remover". This is because when we created the table, we turned on using English stemming (`morphology "stem_en"`).

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
| 1513686608316989452 | 宠物 <b>毛发去除</b> 手套 | 7.990000 |
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
          "title": "宠物毛发去除手套"
        },
        "highlight": {
          "title": [
            "宠物 <b>毛发去除</b> 手套"
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
    echo "文档 ID: ".$doc->getId()."\n";
    echo "文档评分: ".$doc->getScore()."\n";
    echo "文档字段:\n";
    print_r($doc->getData());
    echo "高亮: \n";
    print_r($doc->getHighlight());
}
```
<!-- response PHP -->

```php
文档ID: 1513686608316989452
文档分数: 1680
文档字段:
数组
(
    [价格] => 7.99
    [标题] => 宠物毛发去除手套
)
高亮:
数组
(
    [标题] => 数组
        (
            [0] => 宠物 <b>毛发去除</b> 手套
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
                    u'_source': {u'title': u'Pet Hair Remover Glove', u'price':7.99},
                    u'highlight':{u'title':[u'Pet <b>Hair Remover</b> Glove']}}}],
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
                    "_source": {"title": "Pet Hair Remover Glove", "price":7.99},
                    "highlight":{"title":["Pet <b>Hair Remover</b> Glove"]}}],
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

更多有关Manticore中可用的不同搜索选项的信息可以在我们的学习课程中找到：
* [分面搜索](https://play.manticoresearch.com/faceting/)
* [地理搜索](https://play.manticoresearch.com/geosearch/)
* [搜索相似文档](https://play.manticoresearch.com/mlt/)

<!-- example update -->
## 更新

假设我们现在想更新文档 - 将价格改为18.5。这可以通过过滤任何字段来完成，但通常您知道文档ID并根据此更新某些内容。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
update products set price=18.5 where id = 1513686608316989452;
```
<!-- response SQL -->

```sql
查询成功, 影响行数 1 (0.00 秒)
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
  "result": "已更新"
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

Let's now delete all documents with price lower than 10.

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

