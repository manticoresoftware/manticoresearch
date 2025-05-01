# СОВПАДЕНИЕ
Условие `MATCH` позволяет проводить полнотекстовые поиски в текстовых полях. Входная строка запроса токенизируется с использованием тех же настроек, которые применялись к тексту во время индексации. В дополнение к токенизации входного текста, строка запроса поддерживает ряд [полнотекстовых операторов](../../Searching/Full_text_matching/Operators.md), которые накладывают различные правила на то, как ключевые слова должны обеспечивать действительное совпадение.
Условия полнотекстового совпадения могут комбинироваться с атрибутами [фильтров](../../Searching/Filters.md) в виде логического AND. **Отношения OR между полнотекстовыми совпадениями и атрибутными фильтрами не поддерживаются**.
Запрос на совпадение всегда выполняется первым в процессе фильтрации, за ним следуют [атрибутные фильтры](../../Searching/Filters.md). Атрибутные фильтры применяются к множеству результатов запроса на совпадение. Запрос без условия совпадения называется полным сканированием.
В условии `SELECT` может быть не более одного `MATCH()`.
Используя [синтаксис полнотекстового запроса](../../Searching/Full_text_matching/Operators.md), совпадение выполняется по всем индексированным текстовым полям документа, если только выражение не требует совпадения внутри поля (например, поиск фраз) или не ограничено операторами полей.
При использовании [JOIN](../../Searching/Joining.md) запросов, `MATCH()` может принимать необязательный второй параметр, который указывает, к какой таблице должен быть применен полнотекстовый поиск. По умолчанию полнотекстовый запрос применяется к левой таблице в операции `JOIN`:
```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('search query', table2);
```
Это позволяет выполнять полнотекстовые поиски по конкретным таблицам в операции соединения. Для получения более подробной информации о использовании MATCH с JOIN, см. раздел [Объединение таблиц](../../Searching/Joining.md).
## SQL
<!-- example Example_1 -->
```sql
MATCH('search query' [, table_name])
```
- `'search query'`: полнотекстовая строка запроса, которая может включать различные [полнотекстовые операторы](../../Searching/Full_text_matching/Operators.md).
- `table_name`: (необязательный) имя таблицы, к которой следует применить полнотекстовый поиск, используемое в запросах `JOIN` для указания таблицы, отличной от таблицы по умолчанию.
Утверждение [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) использует условие [MATCH](../../Searching/Full_text_matching/Basic_usage.md), которое должно находиться после WHERE, для выполнения полнотекстовых поисков. `MATCH()` принимает входную строку, в которой доступны все [полнотекстовые операторы](../../Searching/Full_text_matching/Operators.md).
<!-- intro -->
##### SQL:
<!-- request SQL -->
```sql
SELECT * FROM myindex WHERE MATCH('"find me fast"/2');
```
<!-- response SQL -->
```sql
+------+------+----------------+
| id   | gid  | title          |
+------+------+----------------+
|    1 |   11 | first find me  |
|    2 |   12 | second find me |
+------+------+----------------+
2 строки в наборе (0.00 сек)
```
<!-- request MATCH with filters -->
Пример более сложного запроса с использованием MATCH и WHERE фильтров.
```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```
<!-- end -->
## HTTP JSON
<!-- example Example_11 -->
Полнотекстовое совпадение доступно в конечной точке `/search` и в клиентских приложениях на основе HTTP. Следующие условия могут использоваться для выполнения полнотекстовых совпадений:
### match
"match" - это простой запрос, который совпадает с указанными ключевыми словами в указанных полях.
```json
"query":
{
  "match": { "field": "keyword" }
}
```
Вы можете указать список полей:
```json
"match":
{
  "field1,field2": "keyword"
}
```
Или вы можете использовать `_all` или `*` для поиска по всем полям.
Вы можете искать по всем полям, за исключением одного, используя "!field":
```json
"match":
{
  "!field1": "keyword"
}
```
По умолчанию ключевые слова комбинируются с использованием оператора OR. Однако вы можете изменить это поведение с помощью условия "оператор":
```json
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
```
"оператор" может быть установлен на "or" или "and".
Модификатор `boost` также может быть применен. Он повышает слово [IDF](../../Searching/Options.md#idf)_score на указанный коэффициент в оценках ранжирования, которые учитывают IDF в своих расчетах. Он не влияет на процесс совпадения каким-либо образом.
```json
"query":
{
  "match":
  {
    "field1":
    {
      "query": "keyword",
      "boost": 2.0
    }
  }
}
```
### match_phrase
"match_phrase" - это запрос, который совпадает с целой фразой. Он похож на оператор фраз в SQL. Вот пример:
```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```
### query_string
"query_string" принимает входную строку в качестве полнотекстового запроса в синтаксисе `MATCH()`.
```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```
### match_all
"match_all" принимает пустой объект и возвращает документы из таблицы без выполнения каких-либо атрибутных фильтров или полнотекстовых совпадений. В качестве альтернативы, вы можете просто опустить условие `query` в запросе, что приведет к такому же эффекту.
```json
"query":
{
  "match_all": {}
}
```
### Комбинирование полнотекстового фильтра с другими фильтрами
Все условия полнотекстового совпадения могут комбинироваться с операторами [must](../../Searching/Filters.md#must), [must_not](../../Searching/Filters.md#must_not) и [should](../../Searching/Filters.md#should) JSON `bool` запроса (../../Searching/Filters.md#bool-query).
<!-- intro -->
Примеры:
<!-- request match -->
```json
POST /search
-d
'{ 
    "table" : "hn_small",
    "query":
    {
        "match": 
        {
            "*" : "найти джо"
        }
    },
    "_source": ["story_author","comment_author"],
    "limit": 1
}'
```

<!-- response match -->
```json
{
   "took" : 3,
   "timed_out" : false,
   "hits" : {
      "hits" : [
         {
            "_id": 668018,
            "_score" : 3579,
            "_source" : {
               "story_author" : "IgorPartola",
               "comment_author" : "joe_the_user"
            }
         }
      ],
      "total" : 88063,
      "total_relation" : "eq"
   }
}
```
<!-- request match_phrase -->
```json
POST /search
-d
'{
    "table" : "hn_small",
    "query":
    {
        "match_phrase":
        {
            "*" : "найти джо"
        }
    },
    "_source": ["story_author","comment_author"],
    "limit": 1
}'
```
<!-- response match_phrase -->
```json
{
   "took" : 3,
   "timed_out" : false,
   "hits" : {
      "hits" : [
         {
            "_id": 807160,
            "_score" : 2599,
            "_source" : {
               "story_author" : "rbanffy",
               "comment_author" : "runjake"
            }
         }
      ],
      "total" : 2,
      "total_relation" : "eq"
   }
}
```

<!-- request query_string -->
```json
POST /search
-d
'{   "table" : "hn_small",
    "query":
    {
        "query_string": "@comment_text "найти джо быстро "/2"
    },
    "_source": ["story_author","comment_author"],
    "limit": 1
}'
```
<!-- response query_string -->
```json
{
  "took" : 3,
  "timed_out" : false,
  "hits" : {
      "hits" : [
         {
            "_id": 807160,
            "_score" : 2566,
            "_source" : {
               "story_author" : "rbanffy",
               "comment_author" : "runjake"
            }
         }
      ],
      "total" : 1864,
      "total_relation" : "eq"
   }
}
```
<!-- request PHP -->
```php
$search = new Search(new Client());
$result = $search->('@title найди меня быстро');
foreach($result as $doc)
{
    echo 'Документ: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
}
```
<!-- response PHP -->
```php
Документ: 1
title: первый найди меня быстро
gid: 11
Документ: 2
title: второй найди меня быстро
gid: 12

```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text "найти джо быстро "/2"}, "_source": ["story_author","comment_author"], "limit":1})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{'_id': '807160',
                    '_score': 2566,
                    '_source': {'comment_author': 'runjake',
                                'story_author': 'rbanffy'}}],
          'max_score': None,
          'total': 1864,
          'total_relation': 'eq'},
 'profile': None,
 'timed_out': False,
 'took': 2,
 'warning': None}
```

<!-- intro -->
Python-asyncio
<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text "найти джо быстро "/2"}, "_source": ["story_author","comment_author"], "limit":1})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{'_id': '807160',
                    '_score': 2566,
                    '_source': {'comment_author': 'runjake',
                                'story_author': 'rbanffy'}}],
          'max_score': None,
          'total': 1864,
          'total_relation': 'eq'},
 'profile': None,
 'timed_out': False,
 'took': 2,
 'warning': None}
```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text "найти джо быстро "/2"}, "_source": ["story_author","comment_author"], "limit":1});
```
<!-- response javascript -->
```javascript
{
  took: 1,
  timed_out: false,
  hits:
   exports {
     total: 1864,
     total_relation: 'eq',
     hits:
      [ { _id: '807160',
          _score: 2566,
          _source: { story_author: 'rbanffy', comment_author: 'runjake' }
        }
      ]
   }
}
```

<!-- intro -->
java
<!-- request Java -->

```java

query = new HashMap<String,Object>();
query.put("query_string", "@comment_text "найти джо быстро "/2");
searchRequest = new SearchRequest();
searchRequest.setIndex("hn_small");
searchRequest.setQuery(query);
searchRequest.addSourceItem("story_author");
searchRequest.addSourceItem("comment_author");
searchRequest.limit(1);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
```java
class SearchResponse {
    took: 1
    timedOut: false
    aggregations: null
    hits: class SearchResponseHits {
        maxScore: null
        total: 1864
        totalRelation: eq
        hits: [{_id=807160, _score=2566, _source={story_author=rbanffy, comment_author=runjake}}]
    }
    profile: null
    warning: null
}
```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { query_string="@comment_text "найти джо быстро "/2" };
var searchRequest = new SearchRequest("hn_small", query);
searchRequest.Source = new List<string> {"story_author", "comment_author"};
searchRequest.Limit = 1;
SearchResponse searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
```clike
class SearchResponse {
    took: 1
    timedOut: false
    aggregations: null
    hits: class SearchResponseHits {
        maxScore: null
        total: 1864
        totalRelation: eq
        hits: [{_id=807160, _score=2566, _source={story_author=rbanffy, comment_author=runjake}}]
    }
    profile: null
    warning: null
}
```

<!-- intro -->
Rust
<!-- request Rust -->

```rust
let query = SearchQuery {
     query_string: Some(serde_json::json!("@comment_text "find joe fast "/2").into()),
    ..Default::default()
};
let search_req = SearchRequest {
    table: "hn_small".to_string(),
    query: Some(Box::new(query)),
    source: serde_json::json!(["story_author", "comment_author"]),
    limit: serde_json::json!(1),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
```rust
class SearchResponse {
    took: 1
    timedOut: false
    aggregations: null
    hits: class SearchResponseHits {
        maxScore: null
        total: 1864
        totalRelation: eq
        hits: [{_id=807160, _score=2566, _source={story_author=rbanffy, comment_author=runjake}}]
    }
    profile: null
    warning: null
}
```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({
  index: 'test',
  query: { query_string: "test document 1" },
  "_source": ["content", "title"],
  limit: 1
});
```
<!-- response TypeScript -->
```json
{
  took: 1,
  timed_out: false,
  hits:
   exports {
     total: 5,
     total_relation: 'eq',
     hits:
      [ { _id: '1',
          _score: 2566,
          _source: { content: 'This is a test document 1', title: 'Doc 1' }
        }
      ]
   }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
searchRequest := manticoresearch.NewSearchRequest("test")
query := map[string]interface{} {"query_string": "test document 1"}
searchReq.SetSource([]string{"content", "title"})
searchReq.SetLimit(1)
resp, httpRes, err := search.SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
```json
{
  "hits": {
    "hits": [
      {
        "_id": 1,
        "_score": 2566,
        "_source": {
          "content": "This is a test document 1",
          "title": "Doc 1"
        }
      }
    ],
    "total": 5,
    "total_relation": "eq"
  },
  "timed_out": false,
  "took": 0
}
```
<!-- end -->
<!-- proofread -->
















































# СОВПАДЕНИЕ
Условие `MATCH` позволяет выполнять полнотекстовые поиски в текстовых полях. Входная строка запроса [токенизируется](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) с использованием тех же настроек, которые применяются к тексту во время индексации. В дополнение к токенизации входного текста строка запроса поддерживает ряд [полнотекстовых операторов](../../Searching/Full_text_matching/Operators.md), которые накладывают различные правила на то, как ключевые слова должны обеспечивать допустимое соответствие.
Полнотекстовые условия соответствия могут комбинироваться с атрибутными [фильтрами](../../Searching/Filters.md) в логическом И. **Логические ИЛИ между полнотекстовыми совпадениями и атрибутными фильтрами не поддерживаются**.
Запрос на соответствие всегда выполняется первым в процессе фильтрации, после чего применяются [атрибутные фильтры](../../Searching/Filters.md). Атрибутные фильтры применяются к набору результатов запроса совпадения. Запрос без условия совпадения называется полным сканированием.
В условии `SELECT` должно быть не более одного `MATCH()`.
Используя [синтаксис полнотекстового запроса](../../Searching/Full_text_matching/Operators.md), соответствие выполняется по всем индексированным текстовым полям документа, если только выражение не требует совпадения внутри поля (например, при поиске фраз) или не ограничено полевыми операторами.
При использовании запросов [JOIN](../../Searching/Joining.md) `MATCH()` может принимать необязательный второй параметр, который указывает, к какой таблице должен быть применен полнотекстовый поиск. По умолчанию полнотекстовый запрос применяется к левой таблице в операции `JOIN`:
```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('search query', table2);
```
Это позволяет выполнять полнотекстовые поиски по определенным таблицам в операции соединения. Для получения дополнительной информации о использовании MATCH с JOIN, см. раздел [Соединение таблиц](../../Searching/Joining.md).
## SQL
<!-- example Example_1 -->
```sql
MATCH('search query' [, table_name])
```
- `'search query'`: Строка запроса полнотекстового поиска, которая может включать различные [полнотекстовые операторы](../../Searching/Full_text_matching/Operators.md).
- `table_name`: (Необязательно) Имя таблицы, к которой применяется полнотекстовый поиск, используется в запросах `JOIN`, чтобы указать другую таблицу, отличную от левой таблицы по умолчанию.
Утверждение [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) использует условие [MATCH](../../Searching/Full_text_matching/Basic_usage.md), которое должно следовать за WHERE, для выполнения полнотекстовых поисков. `MATCH()` принимает входную строку, в которой доступны все [полнотекстовые операторы](../../Searching/Full_text_matching/Operators.md).
<!-- intro -->
##### SQL:
<!-- request SQL -->
```sql
SELECT * FROM myindex WHERE MATCH('"find me fast"/2');
```
<!-- response SQL -->
```sql
+------+------+----------------+
| id   | gid  | title          |
+------+------+----------------+
|    1 |   11 | first find me  |
|    2 |   12 | second find me |
+------+------+----------------+
2 rows in set (0.00 sec)
```
<!-- request MATCH with filters -->
Пример более сложного запроса с использованием MATCH с фильтрами WHERE.
```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```
<!-- end -->
## HTTP JSON
<!-- example Example_11 -->
Полнотекстовое соответствие доступно в конечной точке `/search` и в клиентах на основе HTTP. Для выполнения полнотекстовых совпадений можно использовать следующие условия:
### match
"match" — это простой запрос, который соответствует заданным ключевым словам в указанных полях.
```json
"query":
{
  "match": { "field": "keyword" }
}
```
Вы можете указать список полей:
```json
"match":
{
  "field1,field2": "keyword"
}
```
Или вы можете использовать `_all` или `*`, чтобы искать во всех полях.
Вы можете искать во всех полях, исключая одно, используя "!field":
```json
"match":
{
  "!field1": "keyword"
}
```
По умолчанию ключевые слова комбинируются с использованием оператора ИЛИ. Однако вы можете изменить это поведение, используя условие "operator":
```json
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
```
"operator" может быть установлен в "or" или "and".
Модификатор `boost` также может быть применен. Он увеличивает слово [IDF](../../Searching/Options.md#idf)_score на указанный фактор в результатах ранжирования, которые включают IDF в свои вычисления. Это не влияет на процесс сопоставления никаким образом.
```json
"query":
{
  "match":
  {
    "field1":
    {
      "query": "keyword",
      "boost": 2.0
    }
  }
}
```
### match_phrase
"match_phrase" — это запрос, который соответствует всей фразе. Он аналогичен оператору фраз в SQL. Вот пример:
```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```
### query_string
"query_string" принимает входную строку в качестве полнотекстового запроса в синтаксисе `MATCH()`.
```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```
### match_all
"match_all" принимает пустой объект и возвращает документы из таблицы без выполнения фильтрации атрибутов или полнотекстового совпадения. В качестве альтернативы вы можете просто опустить условие `query` в запросе, что даст тот же эффект.
```json
"query":
{
  "match_all": {}
}
```
### Комбинирование полнотекстовой фильтрации с другими фильтрами
Все полнотекстовые условия совпадения могут быть объединены с операторами [must](../../Searching/Filters.md#must), [must_not](../../Searching/Filters.md#must_not) и [should](../../Searching/Filters.md#should) JSON-запроса `bool` (../../Searching/Filters.md#bool-query).
<!-- intro -->
Примеры:
<!-- request match -->
```json
POST /search
-d
'{
    "table" : "hn_small",
    "query":
    {
        "match":
        {
            "*" : "найти джо"
        }
    },
    "_source": ["story_author","comment_author"],
    "limit": 1
}'
```

<!-- response match -->
```json
{
   "took" : 3,
   "timed_out" : false,
   "hits" : {
      "hits" : [
         {
            "_id": 668018,
            "_score" : 3579,
            "_source" : {
               "story_author" : "IgorPartola",
               "comment_author" : "joe_the_user"
            }
         }
      ],
      "total" : 88063,
      "total_relation" : "eq"
   }
}
```
<!-- request match_phrase -->
```json
POST /search
-d
'{
    "table" : "hn_small",
    "query":
    {
        "match_phrase":
        {
            "*" : "найти джо"
        }
    },
    "_source": ["story_author","comment_author"],
    "limit": 1
}'
```
<!-- response match_phrase -->
```json
{
   "took" : 3,
   "timed_out" : false,
   "hits" : {
      "hits" : [
         {
            "_id": 807160,
            "_score" : 2599,
            "_source" : {
               "story_author" : "rbanffy",
               "comment_author" : "runjake"
            }
         }
      ],
      "total" : 2,
      "total_relation" : "eq"
   }
}
```

<!-- request query_string -->
```json
POST /search
-d
'{   "table" : "hn_small",
    "query":
    {
        "query_string": "@comment_text \"найти джо быстро \"/2"
    },
    "_source": ["story_author","comment_author"],
    "limit": 1
}'
```
<!-- response query_string -->
```json
{
  "took" : 3,
  "timed_out" : false,
  "hits" : {
      "hits" : [
         {
            "_id": 807160,
            "_score" : 2566,
            "_source" : {
               "story_author" : "rbanffy",
               "comment_author" : "runjake"
            }
         }
      ],
      "total" : 1864,
      "total_relation" : "eq"
   }
}
```
<!-- request PHP -->
```php
$search = new Search(new Client());
$result = $search->('@title найти меня быстро');
foreach($result as $doc)
{
    echo 'Документ: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
}
```
<!-- response PHP -->
```php
Document: 1
title: первый найти меня быстро
gid: 11
Document: 2
title: второй найти меня быстро
gid: 12

```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text \"найти джо быстро \"/2"}, "_source": ["story_author","comment_author"], "limit":1})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{'_id': '807160',
                    '_score': 2566,
                    '_source': {'comment_author': 'runjake',
                                'story_author': 'rbanffy'}}],
          'max_score': None,
          'total': 1864,
          'total_relation': 'eq'},
 'profile': None,
 'timed_out': False,
 'took': 2,
 'warning': None}
```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text \"найти джо быстро \"/2"}, "_source": ["story_author","comment_author"], "limit":1});
```
<!-- response javascript -->
```javascript
{
  took: 1,
  timed_out: false,
  hits:
   exports {
     total: 1864,
     total_relation: 'eq',
     hits:
      [ { _id: '807160',
          _score: 2566,
          _source: { story_author: 'rbanffy', comment_author: 'runjake' }
        }
      ]
   }
}
```

<!-- intro -->
java
<!-- request Java -->

```java

query = new HashMap<String,Object>();
query.put("query_string", "@comment_text \"найти джо быстро \"/2");
searchRequest = new SearchRequest();
searchRequest.setIndex("hn_small");
searchRequest.setQuery(query);
searchRequest.addSourceItem("story_author");
searchRequest.addSourceItem("comment_author");
searchRequest.limit(1);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
```java
class SearchResponse {
    took: 1
    timedOut: false
    aggregations: null
    hits: class SearchResponseHits {
        maxScore: null
        total: 1864
        totalRelation: eq
        hits: [{_id=807160, _score=2566, _source={story_author=rbanffy, comment_author=runjake}}]
    }
    profile: null
    warning: null
}
```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { query_string="@comment_text \"найти джо быстро \"/2" };
var searchRequest = new SearchRequest("hn_small", query);
searchRequest.Source = new List<string> {"story_author", "comment_author"};
searchRequest.Limit = 1;
SearchResponse searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
```clike
class SearchResponse {
    took: 1
    timedOut: false
    aggregations: null
    hits: class SearchResponseHits {
        maxScore: null
        total: 1864
        totalRelation: eq
        hits: [{_id=807160, _score=2566, _source={story_author=rbanffy, comment_author=runjake}}]
    }
    profile: null
    warning: null
}
```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({
  index: 'test',
  query: { query_string: "тестовый документ 1" },
  "_source": ["content", "title"],
  limit: 1
});
```
<!-- response TypeScript -->
```json
{
  took: 1,
  timed_out: false,
  hits:
   exports {
     total: 5,
     total_relation: 'eq',
     hits:
      [ { _id: '1',
          _score: 2566,
          _source: { content: 'Это тестовый документ 1', title: 'Док 1' }
        }
      ]
   }
}
```

<!-- intro -->
Идти
<!-- request Go -->

```go
searchRequest := manticoresearch.NewSearchRequest("test")
query := map[string]interface{} {"query_string": "тестовый документ 1"}
searchReq.SetSource([]string{"content", "title"})
searchReq.SetLimit(1)
resp, httpRes, err := search.SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
```json
{
  "hits": {
    "hits": [
      {
        "_id": 1,
        "_score": 2566,
        "_source": {
          "content": "Это тестовый документ 1",
          "title": "Док 1"
        }
      }
    ],
    "total": 5,
    "total_relation": "eq"
  },
  "timed_out": false,
  "took": 0
}
```
<!-- end -->
<!-- proofread -->
















































