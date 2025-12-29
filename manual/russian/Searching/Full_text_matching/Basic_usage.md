# MATCH

Клауза `MATCH` позволяет выполнять полнотекстовый поиск по текстовым полям. Входная строка запроса [токенизируется](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) с использованием тех же настроек, которые применялись к тексту при индексировании. Помимо токенизации входного текста, строка запроса поддерживает ряд [операторов полнотекстового поиска](../../Searching/Full_text_matching/Operators.md), которые накладывают различные правила на то, как ключевые слова должны обеспечивать валидное совпадение.

Клаузы полнотекстового поиска могут быть объединены с атрибутными [фильтрами](../../Searching/Filters.md) посредством логического И. **Отношения ИЛИ между полнотекстовым поиском и атрибутными фильтрами не поддерживаются**.

Запрос на полнотекстовый поиск всегда выполняется первым в процессе фильтрации, за ним следуют [атрибутные фильтры](../../Searching/Filters.md). Атрибутные фильтры применяются к результирующему набору запроса полнотекстового поиска. Запрос без клаузы `MATCH` называется полным сканированием (fullscan).

В клаузе `SELECT` может быть не более одного `MATCH()`.

Используя [синтаксис полнотекстового запроса](../../Searching/Full_text_matching/Operators.md), поиск выполняется по всем индексированным текстовым полям документа, если выражение не требует совпадения внутри конкретного поля (как при поиске по фразе) или не ограничено операторами полей.

При использовании запросов с [JOIN](../../Searching/Joining.md), `MATCH()` может принимать необязательный второй параметр, который указывает, к какой таблице следует применить полнотекстовый поиск. По умолчанию полнотекстовый запрос применяется к левой таблице в операции `JOIN`:

```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('search query', table2);
```

Это позволяет выполнять полнотекстовый поиск по конкретным таблицам в операции соединения. Для получения более подробной информации об использовании MATCH с JOIN, см. раздел [Соединение таблиц](../../Searching/Joining.md).

## SQL

<!-- example Example_1 -->

```sql
MATCH('search query' [, table_name])
```
- `'search query'`: Строка запроса полнотекстового поиска, которая может включать различные [операторы полнотекстового поиска](../../Searching/Full_text_matching/Operators.md).
- `table_name`: (Необязательно) Имя таблицы, к которой следует применить полнотекстовый поиск, используется в запросах `JOIN` для указания таблицы, отличной от таблицы по умолчанию (левой).


Оператор [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) использует клаузу [MATCH](../../Searching/Full_text_matching/Basic_usage.md), которая должна следовать после WHERE, для выполнения полнотекстового поиска. `MATCH()` принимает входную строку, в которой доступны все [операторы полнотекстового поиска](../../Searching/Full_text_matching/Operators.md).


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
Пример более сложного запроса с использованием MATCH вместе с фильтрами WHERE.

```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```

<!-- end -->

## HTTP JSON

<!-- example Example_11 -->

Полнотекстовый поиск доступен в эндпоинте `/search` и в HTTP-клиентах. Для выполнения полнотекстового поиска могут использоваться следующие клаузы:

### match

"match" — это простой запрос, который ищет указанные ключевые слова в указанных полях.

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

Вы можете искать по всем полям, кроме одного, используя "!field":

```json
"match":
{
  "!field1": "keyword"
}
```
По умолчанию ключевые слова объединяются с помощью оператора ИЛИ. Однако это поведение можно изменить с помощью клаузы "operator":

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

Также может быть применен модификатор `boost`. Он увеличивает [IDF](../../Searching/Options.md#idf)_score слова на указанный коэффициент в ранжирующих оценках, которые включают IDF в свои расчеты. Это никак не влияет на процесс поиска совпадений.
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

"match_phrase" — это запрос, который ищет точное совпадение всей фразы. Он аналогичен оператору фразы в SQL. Вот пример:

```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```

### query_string
"query_string" принимает входную строку как полнотекстовый запрос в синтаксисе `MATCH()`.

```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```

### match_all

"match_all" принимает пустой объект и возвращает документы из таблицы без выполнения какого-либо атрибутного фильтрования или полнотекстового поиска. Альтернативно, вы можете просто опустить клаузу `query` в запросе, что даст тот же эффект.

```json
"query":
{
  "match_all": {}
}
```


### Комбинирование полнотекстового фильтра с другими фильтрами

Все клаузы полнотекстового поиска могут быть объединены с операторами [must](../../Searching/Filters.md#must), [must_not](../../Searching/Filters.md#must_not) и [should](../../Searching/Filters.md#should) [JSON `bool` запроса](../../Searching/Filters.md#bool-query).

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

