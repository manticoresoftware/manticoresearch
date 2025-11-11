# MATCH

Клауза `MATCH` позволяет выполнять полнотекстовый поиск в текстовых полях. Входящая строка запроса [токенизируется](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) с использованием тех же настроек, которые применялись к тексту при индексировании. Помимо токенизации входного текста, строка запроса поддерживает ряд [операторов полнотекстового поиска](../../Searching/Full_text_matching/Operators.md), которые задают различные правила, с помощью которых ключевые слова должны обеспечивать валидное совпадение.

Клаузы полнотекстового сопоставления могут комбинироваться с [фильтрами](../../Searching/Filters.md) по атрибутам, используя логическое И (AND). **Отношения ИЛИ между полнотекстовыми совпадениями и атрибутными фильтрами не поддерживаются**.

Запрос сопоставления всегда выполняется первым на этапе фильтрации, а затем применяются [фильтры атрибутов](../../Searching/Filters.md). Фильтры атрибутов применяются к результату запроса сопоставления. Запрос без клаузы match называется fullscan.

В клауза `SELECT` может присутствовать не более одной `MATCH()`.

Используя [синтаксис полнотекстовых запросов](../../Searching/Full_text_matching/Operators.md), сопоставление выполняется по всем проиндексированным текстовым полям документа, если только выражение не требует совпадения в определённом поле (например, поиск фразы) или не ограничено операторами полей.

При использовании запросов с [JOIN](../../Searching/Joining.md), `MATCH()` может принимать необязательный второй параметр, который указывает таблицу, к которой следует применить полнотекстовый поиск. По умолчанию полнотекстовый запрос применяется к левой таблице в операции `JOIN`:

```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('search query', table2);
```

Это позволяет выполнять полнотекстовый поиск по конкретным таблицам в операции объединения. Для подробностей использования MATCH с JOIN смотрите раздел [Объединение таблиц](../../Searching/Joining.md).

## SQL

<!-- example Example_1 -->

```sql
MATCH('search query' [, table_name])
```
- `'search query'`: Строка полнотекстового поискового запроса, которая может включать различные [операторы полнотекстового поиска](../../Searching/Full_text_matching/Operators.md).
- `table_name`: (необязательно) Имя таблицы, к которой применяется полнотекстовый поиск, используется в запросах с `JOIN` для указания таблицы, отличной от левой по умолчанию.


Оператор [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) использует клауза [MATCH](../../Searching/Full_text_matching/Basic_usage.md), который должен идти после WHERE, для выполнения полнотекстовых поисков. `MATCH()` принимает входную строку, в которой доступны все [операторы полнотекстового поиска](../../Searching/Full_text_matching/Operators.md).


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
Пример более сложного запроса с использованием MATCH и фильтров WHERE.

```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```

<!-- end -->

## HTTP JSON

<!-- example Example_11 -->

Полнотекстовое сопоставление доступно через эндпоинт `/search` и в HTTP-клиентах. Для полнотекстовых сопоставлений можно использовать следующие клаузы:

### match

"match" — простой запрос, который сопоставляет указанные ключевые слова в заданных полях.

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
Или использовать `_all` или `*` для поиска по всем полям.

Можно искать по всем полям, кроме одного, используя "!field":

```json
"match":
{
  "!field1": "keyword"
}
```
По умолчанию ключевые слова объединяются оператором OR. Однако это поведение можно изменить с помощью клаузы "operator":

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

Также можно использовать модификатор `boost`. Он увеличивает значение [IDF](../../Searching/Options.md#idf)_оценки слова на указанный коэффициент в ранжировочных оценках, которые учитывают IDF при вычислениях. Это не влияет на процесс сопоставления.
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

"match_phrase" — запрос, который сопоставляет всю фразу целиком. Это похоже на оператор фразы в SQL. Пример:

```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```

### query_string
"query_string" принимает входную строку в синтаксисе `MATCH()` и использует ее как полнотекстовый запрос.

```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```

### match_all

"match_all" принимает пустой объект и возвращает документы из таблицы без применения фильтров по атрибутам или полнотекстового сопоставления. Альтернативно можно просто опустить клауза `query` в запросе — эффект будет тот же.

```json
"query":
{
  "match_all": {}
}
```


### Комбинирование полнотекстового фильтра с другими фильтрами

Все клаузы полнотекстового сопоставления можно комбинировать с операторами [must](../../Searching/Filters.md#must), [must_not](../../Searching/Filters.md#must_not) и [should](../../Searching/Filters.md#should) JSON `bool` запроса [bool](../../Searching/Filters.md#bool-query).

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

