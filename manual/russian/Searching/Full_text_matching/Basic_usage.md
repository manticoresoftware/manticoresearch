# MATCH

Клауза `MATCH` позволяет выполнять полнотекстовый поиск в текстовых полях. Входящая строка запроса [токенизируется](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) с использованием тех же настроек, что применяются к тексту при индексировании. Помимо токенизации входного текста, строка запроса поддерживает ряд [операторов полнотекстового поиска](../../Searching/Full_text_matching/Operators.md), которые задают различные правила того, как ключевые слова должны обеспечивать валидное совпадение.

Клаузы полнотекстового поиска могут комбинироваться с [фильтрами атрибутов](../../Searching/Filters.md) через булев оператор AND. **Отношения OR между полнотекстовыми поисками и фильтрами атрибутов не поддерживаются**.

Запрос match всегда выполняется первым в процессе фильтрации, за ним следуют [фильтры атрибутов](../../Searching/Filters.md). Фильтры атрибутов применяются к результирующему набору от запроса match. Запрос без клаузы match называется fullscan.

В `SELECT` должно быть не более одного вызова `MATCH()`.

Используя [синтаксис полнотекстового запроса](../../Searching/Full_text_matching/Operators.md), поиск выполняется по всем индексированным текстовым полям документа, если только выражение не требует совпадения в конкретном поле (например, поиск фраз) или не ограничено операторами поля.

При использовании [JOIN](../../Searching/Joining.md) запросов, `MATCH()` может принимать необязательный второй параметр, указывающий, к какой таблице следует применить полнотекстовый поиск. По умолчанию полнотекстовый запрос применяется к левой таблице в операции JOIN:

```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('search query', table2);
```

Это позволяет выполнять полнотекстовый поиск по конкретным таблицам в операции объединения. Для подробностей использования MATCH с JOIN смотрите раздел [Объединение таблиц](../../Searching/Joining.md).

## SQL

<!-- example Example_1 -->

```sql
MATCH('search query' [, table_name])
```
- `'search query'`: строка полнотекстового поискового запроса, которая может включать различные [операторы полнотекстового поиска](../../Searching/Full_text_matching/Operators.md).
- `table_name`: (необязательно) имя таблицы, к которой применять полнотекстовый поиск, используется в JOIN запросах для указания другой таблицы вместо таблицы по умолчанию слева.


Выражение [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) использует клауза [MATCH](../../Searching/Full_text_matching/Basic_usage.md), которая должна идти после WHERE и используется для полнотекстового поиска. `MATCH()` принимает входную строку, в которой доступны все [операторы полнотекстового поиска](../../Searching/Full_text_matching/Operators.md).


<!-- intro -->
##### SQL:


<!--
data for the following example:

DROP TABLE IF EXISTS myindex;
CREATE TABLE myindex(gid int, title text);
INSERT INTO myindex(gid, title) VALUES
(11, 'first find me'),
(12, 'second find me');
-->

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

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT * FROM myindex WHERE MATCH('"find me fast"/2');"
```
<!-- response JSON -->

```JSON
[
  {
    "columns": [
      {
        "id": {
          "type": "long long"
        }
      },
      {
        "gid": {
          "type": "long"
        }
      },
      {
        "title": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "id": 1,
        "gid": 11,
        "title": "first find me"
      },
      {
        "id": 1,
        "gid": 12,
        "title": "second find me"
      },
    ],
    "total": 2,
    "error": "",
    "warning": ""
  }
]
```

<!-- request MATCH with filters -->
Пример более сложного запроса с использованием MATCH вместе с фильтрами WHERE.

```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```

<!-- end -->

## HTTP JSON

<!-- example Example_11 -->

Полнотекстовый поиск доступен на эндпоинте `/search` и в HTTP-клиентах. Для выполнения полнотекстового поиска можно использовать следующие клаузы:

### match

"match" — это простой запрос, который ищет указанные ключевые слова в указанных полях.

```json
"query":
{
  "match": { "field": "keyword" }
}
```

Можно указать список полей:

```json
"match":
{
  "field1,field2": "keyword"
}
```
Или можно использовать `_all` или `*` для поиска по всем полям.

Можно выполнить поиск по всем полям, кроме одного, используя "!field":

```json
"match":
{
  "!field1": "keyword"
}
```
По умолчанию ключевые слова объединяются оператором OR. Однако можно изменить это поведение, используя параметр "operator":

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

"operator" может принимать значения "or" или "and".

Также может применяться модификатор `boost`. Он увеличивает значение [IDF](../../Searching/Options.md#idf)_скоринга слова на указанный коэффициент в рейтинговых оценках, где используется IDF. Это не влияет на процесс совпадения.
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

"match_phrase" — это запрос, который ищет полную фразу. Он похож на оператор фразы в SQL. Пример:

```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```

### query_string
"query_string" принимает входную строку в синтаксисе `MATCH()` для полнотекстового запроса.

```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```

### match_all

"match_all" принимает пустой объект и возвращает документы из таблицы без применения фильтров атрибутов или полнотекстового поиска. Альтернативно можно просто опустить клауза `query` в запросе — эффект будет тот же.

```json
"query":
{
  "match_all": {}
}
```


### Комбинирование полнотекстовой фильтрации с другими фильтрами

Все клаузы полнотекстового поиска можно комбинировать с операторами [must](../../Searching/Filters.md#must), [must_not](../../Searching/Filters.md#must_not) и [should](../../Searching/Filters.md#should) JSON-запроса типа `bool` ([bool query](../../Searching/Filters.md#bool-query)).

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
            "*" : "find joe"
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
            "*" : "find joe"
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
        "query_string": "@comment_text \"find joe fast \"/2"
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
$result = $search->('@title find me fast');
foreach($result as $doc)
{
    echo 'Document: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
}
```
<!-- response PHP -->
```php
Document: 1
title: first find me fast
gid: 11
Document: 2
title: second find me fast
gid: 12

```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text \"find joe fast \"/2"}, "_source": ["story_author","comment_author"], "limit":1})
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
await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text \"find joe fast \"/2"}, "_source": ["story_author","comment_author"], "limit":1})
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
res = await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text \"find joe fast \"/2"}, "_source": ["story_author","comment_author"], "limit":1});
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
query.put("query_string", "@comment_text \"find joe fast \"/2");
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
object query =  new { query_string="@comment_text \"find joe fast \"/2" };
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
     query_string: Some(serde_json::json!("@comment_text \"find joe fast \"/2").into()),
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

