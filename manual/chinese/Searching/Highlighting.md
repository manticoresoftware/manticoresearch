# 高亮显示

<!-- example highlighting -->

高亮显示使您能够从包含匹配关键词的文档中获取高亮的文本片段（称为片段）。

SQL 中的 `HIGHLIGHT()` 函数，通过 HTTP 的 JSON 查询中的 `"highlight"` 属性，以及 PHP 客户端中的 `highlight()` 函数，均利用内置的文档存储来检索原始字段内容（默认启用）。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT() FROM books WHERE MATCH('try');
```

<!-- response SQL -->

```sql
+----------------------------------------------------------+
| highlight()                                              |
+----------------------------------------------------------+
| Don`t <b>try</b> to compete in childishness, said Bliss. |
+----------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query":  {  "match": { "*" : "try" }  },
  "highlight": {}
}
```

<!-- response JSON -->

```json
{
  "took":1,
  "timed_out":false,
  "hits":
  {
    "total":1,
    "hits":
    [
      {
        "_id": 4,
        "_score":1704,
        "_source":
        {
          "title":"Book four",
          "content":"Don`t try to compete in childishness, said Bliss."
        },
        "highlight":
        {
          "title": ["Book four"],
          "content": ["Don`t <b>try</b> to compete in childishness, said Bliss."]
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
$results = $index->search('try')->highlight()->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}

```

<!-- response PHP -->
```php
Document: 14
title: Book four
content: Don`t try to compete in childishness, said Bliss.
Highlight for title:
- Book four
Highlight for content:
- Don`t <b>try</b> to compete in childishness, said Bliss.

```

<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"try"}},"highlight":{}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'4',
                    u'_score': 1695,
                    u'_source': {u'content': u'Don`t try to compete in childishness, said Bliss.',
                                 u'title': u'Book four'},
                    u'highlight': {u'content': [u'Don`t <b>try</b> to compete in childishness, said Bliss.'],
                                   u'title': [u'Book four']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"try"}},"highlight":{}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'4',
                    u'_score': 1695,
                    u'_source': {u'content': u'Don`t try to compete in childishness, said Bliss.',
                                 u'title': u'Book four'},
                    u'highlight': {u'content': [u'Don`t <b>try</b> to compete in childishness, said Bliss.'],
                                   u'title': [u'Book four']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"try"}},"highlight":{}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 4,"_score":1695,"_source":{"title":"Book four","content":"Don`t try to compete in childishness, said Bliss."},"highlight":{"title":["Book four"],"content":["Don`t <b>try</b> to compete in childishness, said Bliss."]}}]}}
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","try|gets|down|said");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{

}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- response Java -->
```java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 3
        maxScore: null
        hits: [{_id=3, _score=1597, _source={title=Book three, content=Trevize whispered, "It gets infantile pleasure out of display. I`d love to knock it down."}, highlight={title=[Book three], content=[, "It <b>gets</b> infantile pleasure ,  to knock it <b>down</b>."]}}, {_id=4, _score=1563, _source={title=Book four, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[Book four], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=Books two, content=A door opened before them, revealing a small room. Bander said, "Come, half-humans, I want to show you how we live."}, highlight={title=[Books two], content=[ a small room. Bander <b>said</b>, "Come, half-humans, I]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "try|gets|down|said");
var highlight = new Highlight();
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);
```

<!-- response C# -->
```clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 3
        maxScore: null
        hits: [{_id=3, _score=1597, _source={title=Book three, content=Trevize whispered, "It gets infantile pleasure out of display. I`d love to knock it down."}, highlight={title=[Book three], content=[, "It <b>gets</b> infantile pleasure ,  to knock it <b>down</b>."]}}, {_id=4, _score=1563, _source={title=Book four, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[Book four], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=Books two, content=A door opened before them, revealing a small room. Bander said, "Come, half-humans, I want to show you how we live."}, highlight={title=[Books two], content=[ a small room. Bander <b>said</b>, "Come, half-humans, I]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->
```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "try|gets|down|said".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};
let highlight = Highlight::new();

let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
```

<!-- response Rust -->
```rust
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 3
        maxScore: null
        hits: [{_id=3, _score=1597, _source={title=Book three, content=Trevize whispered, "It gets infantile pleasure out of display. I`d love to knock it down."}, highlight={title=[Book three], content=[, "It <b>gets</b> infantile pleasure ,  to knock it <b>down</b>."]}}, {_id=4, _score=1563, _source={title=Book four, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[Book four], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=Books two, content=A door opened before them, revealing a small room. Bander said, "Come, half-humans, I want to show you how we live."}, highlight={title=[Books two], content=[ a small room. Bander <b>said</b>, "Come, half-humans, I]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'Text 1'
    }
  },
  highlight: {}
});
```
<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1"
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1"
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

当使用 SQL 进行搜索结果高亮时，由于 MySQL 协议的限制，您将从多个字段接收合并为单个字符串的片段。您可以通过下面详细说明的 `field_separator` 和 `snippet_separator` 选项调整拼接分隔符。

当通过 HTTP 执行 JSON 查询或使用 PHP 客户端时，没有此类限制，结果集将包含一个字段数组，每个字段包含片段数组（无分隔符）。

请注意，生成片段的选项如 `limit`、`limit_words` 和 `limit_snippets` 默认是针对每个字段单独生效的。您可以通过 `limits_per_field` 选项更改此行为，但这可能导致不希望出现的结果。例如，某个字段有匹配的关键词，但因高亮引擎中该字段的片段排名不如其他字段高，导致该字段的片段未包含在结果集中。

当前的高亮算法优先考虑更好的片段（具有更接近的短语匹配），其次是包含当前结果中尚未出现的关键词的片段。通常，它旨在高亮查询的最佳匹配，并高亮全部查询关键词（受限制条件允许）。如果当前字段没有匹配，则根据限制剪裁文档开头并返回默认内容。若想返回空字符串，请将 `allow_empty` 选项设置为 1。

高亮是在所谓的 `post limit` 阶段执行的，这意味着片段生成被推迟，不仅直到整个最终结果集准备好，而且在应用 LIMIT 子句之后。例如，使用 LIMIT 20,10 子句时，`HIGHLIGHT()` 函数最多会被调用 10 次。

## 高亮选项

<!-- example highlighting options -->

有若干可选高亮选项可用于微调片段生成，这些选项在 SQL、HTTP 和 PHP 客户端中通用。

#### before_match
插入到关键词匹配前的字符串。此字符串中可以使用 `%SNIPPET_ID%` 宏。宏的第一次出现会被替换成当前片段内递增的片段编号。编号默认从 1 开始，但可用 `start_snippet_id` 选项覆盖。%SNIPPET_ID% 在每个新文档开头重新计数。默认值是 `<b>`。

#### after_match
插入到关键词匹配后的字符串。默认值是 `</b>`。

#### limit
片段最大大小，单位为符号（代码点）。默认值是 256。默认情况下此限制针对每个字段单独应用，具体见 `limits_per_field`。

#### limit_words
限制结果中包含的最大单词数。注意此限制应用于所有单词，而不仅仅是要高亮的匹配关键词。例如，高亮 `Mary`，而选中的片段为 `Mary had a little lamb`，这将计入 5 个单词，而非仅 1。默认值是 0（无限制）。默认情况下此限制针对每个字段单独应用，具体见 `limits_per_field`。

#### limit_snippets
限制结果中包含的最大片段数。默认值是 0（无限制）。默认情况下此限制针对每个字段单独应用，具体见 `limits_per_field`。

#### limits_per_field
决定 `limit`、`limit_words` 和 `limit_snippets` 是在每个字段内分别独立限制，还是对整个文档作为全局限制。将此选项设置为 0 表示一个文档的所有合并高亮结果必须在指定限制内。缺点是如果高亮引擎判断某些片段更相关，某字段可能突出显示多个片段，而另一个字段可能没有片段。默认值是 1（使用每字段限制）。
#### around
每个匹配关键词块周围选择的单词数。默认值是 5。

#### use_boundaries
决定是否通过在表设置中使用 [phrase_boundary](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary) 指令配置的短语边界字符，额外拆分片段。默认值是 0（不使用边界）。

#### weight_order
指定是否按相关性（权重递减）排序提取的片段，或者按在文档中出现的顺序（位置递增）排序。默认值是 0（不使用权重排序）。

#### force_all_words
忽略长度限制，直到结果包含所有关键词。默认值是 0（不强制包含所有关键词）。

#### start_snippet_id
设置 `%SNIPPET_ID%` 宏的起始值（该宏会在 `before_match`、`after_match` 字符串中被检测并展开）。默认值是 1。

#### html_strip_mode
定义HTML剥离模式设置。默认为`index`，表示将使用表设置。其他值包括`none`和`strip`，它们会强制跳过或应用剥离，无视表设置；以及`retain`，它会保留HTML标记并保护其不被高亮。`retain`模式仅在高亮完整文档时可用，因此要求不设置任何片段大小限制。允许的字符串值为`none`、`strip`、`index`和`retain`。

#### allow_empty
当当前字段无法生成任何片段（无关键词匹配或无片段符合限制）时，允许返回空字符串作为高亮结果。默认情况下，将返回原始文本的开头而不是空字符串。默认值为0（不允许空结果）。

#### snippet_boundary
确保片段不跨越句子、段落或区域边界（当与启用了相应索引设置的表一起使用时）。允许的值为`sentence`、`paragraph`和`zone`。

#### emit_zones
在每个片段前输出一个包含包围区域名称的HTML标签。默认值为0（不输出区域名称）。

#### force_snippets
决定即使限制允许高亮整个文本时，是否强制生成片段。默认值为0（不强制生成片段）。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT HIGHLIGHT({limit=50}) FROM books WHERE MATCH('try|gets|down|said');
```

<!-- response SQL -->
```sql
+---------------------------------------------------------------------------+
| highlight({limit=50})                                                     |
+---------------------------------------------------------------------------+
|  ... , "It <b>gets</b> infantile pleasure  ...  to knock it <b>down</b>." |
| Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.           |
|  ...  a small room. Bander <b>said</b>, "Come, half-humans, I ...         |
+---------------------------------------------------------------------------+
3 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": {"query_string": "try|gets|down|said"},
  "highlight": { "limit":50 }
}
```

<!-- response JSON -->

```json
{
  "took":2,
  "timed_out":false,
  "hits":
  {
    "total":3,
    "hits":
    [
      {
        "_id": 3,
        "_score":1602,
        "_source":
        {
          "title":"Book three",
          "content":"Trevize whispered, \"It gets infantile pleasure out of display. I`d love to knock it down.\""
        },
        "highlight":
        {
          "title":
          [
            "Book three"
          ],
          "content":
          [
            ", \"It <b>gets</b> infantile pleasure ",
            " to knock it <b>down</b>.\""
          ]
        }
      },
      {
        "_id": 4,
        "_score":1573,
        "_source":
        {
          "title":"Book four",
          "content":"Don`t try to compete in childishness, said Bliss."
        },
        "highlight":
        {
          "title":
          [
            "Book four"
          ],
          "content":
          [
            "Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss."
          ]
        }
      },
      {
        "_id": 2,
        "_score":1521,
        "_source":
        {
          "title":"Book two",
          "content":"A door opened before them, revealing a small room. Bander said, \"Come, half-humans, I want to show you how we live.\""
        },
        "highlight":
        {
          "title":
          [
            "Book two"
          ],
          "content":
          [
            " a small room. Bander <b>said</b>, \"Come, half-humans, I"
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
$results = $index->search('try|gets|down|said')->highlight([],['limit'=>50])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo  $snippet."\n";
        }
    }
}
```

<!-- response PHP -->
```php
Document: 3
title: Book three
content: Trevize whispered, "It gets infantile pleasure out of display. I`d love to knock it down."
Highlight for title:
- Book four
Highlight for content:
, "It <b>gets</b> infantile pleasure
to knock it <b>down</b>."

Document: 4
title: Book four
content: Don`t try to compete in childishness, said Bliss.
Highlight for title:
- Book four
Highlight for content:
Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.

Document: 2
title: Book two
content: A door opened before them, revealing a small room. Bander said, "Come, half-humans, I want to show you how we live.
Highlight for title:
- Book two
Highlight for content:
 a small room. Bander <b>said</b>, \"Come, half-humans, I
```

<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"try"}},"highlight":{"limit":50}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'4',
                    u'_score': 1695,
                    u'_source': {u'content': u'Don`t try to compete in childishness, said Bliss.',
                                 u'title': u'Book four'},
                    u'highlight': {u'content': [u'Don`t <b>try</b> to compete in childishness, said Bliss.'],
                                   u'title': [u'Book four']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"try"}},"highlight":{"limit":50}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'4',
                    u'_score': 1695,
                    u'_source': {u'content': u'Don`t try to compete in childishness, said Bliss.',
                                 u'title': u'Book four'},
                    u'highlight': {u'content': [u'Don`t <b>try</b> to compete in childishness, said Bliss.'],
                                   u'title': [u'Book four']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"query_string":"try|gets|down|said"},"highlight":{"limit":50}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":3,"hits":[{"_id": 3,"_score":1597,"_source":{"title":"Book three","content":"Trevize whispered, \"It gets infantile pleasure out of display. I`d love to knock it down.\""},"highlight":{"title":["Book three"],"content":[", \"It <b>gets</b> infantile pleasure "," to knock it <b>down</b>.\""]}},{"_id": 4,"_score":1563,"_source":{"title":"Book four","content":"Don`t try to compete in childishness, said Bliss."},"highlight":{"title":["Book four"],"content":["Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss."]}},{"_id": 5,"_score":1514,"_source":{"title":"Books two","content":"A door opened before them, revealing a small room. Bander said, \"Come, half-humans, I want to show you how we live.\""},"highlight":{"title":["Books two"],"content":[" a small room. Bander <b>said</b>, \"Come, half-humans, I"]}}]}}
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","try|gets|down|said");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
    put("limit",50);
}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- response Java -->
```java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 3
        maxScore: null
        hits: [{_id=3, _score=1597, _source={title=Book three, content=Trevize whispered, "It gets infantile pleasure out of display. I`d love to knock it down."}, highlight={title=[Book three], content=[, "It <b>gets</b> infantile pleasure ,  to knock it <b>down</b>."]}}, {_id=4, _score=1563, _source={title=Book four, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[Book four], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=Books two, content=A door opened before them, revealing a small room. Bander said, "Come, half-humans, I want to show you how we live."}, highlight={title=[Books two], content=[ a small room. Bander <b>said</b>, "Come, half-humans, I]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "try|gets|down|said");
var highlight = new Highlight();
highlight.Limit = 50;
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);
```

<!-- response C# -->
```clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 3
        maxScore: null
        hits: [{_id=3, _score=1597, _source={title=Book three, content=Trevize whispered, "It gets infantile pleasure out of display. I`d love to knock it down."}, highlight={title=[Book three], content=[, "It <b>gets</b> infantile pleasure ,  to knock it <b>down</b>."]}}, {_id=4, _score=1563, _source={title=Book four, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[Book four], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=Books two, content=A door opened before them, revealing a small room. Bander said, "Come, half-humans, I want to show you how we live."}, highlight={title=[Books two], content=[ a small room. Bander <b>said</b>, "Come, half-humans, I]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "try|gets|down|said".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};
let highlight = Highlight {
    limit: Some(50),
    ..Default::default(),
};

let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
```

<!-- response Rust -->
```rust
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 3
        maxScore: null
        hits: [{_id=3, _score=1597, _source={title=Book three, content=Trevize whispered, "It gets infantile pleasure out of display. I`d love to knock it down."}, highlight={title=[Book three], content=[, "It <b>gets</b> infantile pleasure ,  to knock it <b>down</b>."]}}, {_id=4, _score=1563, _source={title=Book four, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[Book four], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=Books two, content=A door opened before them, revealing a small room. Bander said, "Come, half-humans, I want to show you how we live."}, highlight={title=[Books two], content=[ a small room. Bander <b>said</b>, "Come, half-humans, I]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: { match: { *: 'Text } },
  highlight: { limit: 2}
});
```
<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":2,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		},
		{
			"_id": 2,
			"_score":1480,
			"_source":
			{
				"content":"Text 2",
				"name":"Doc 2",
				"cat":2
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 2</b>"
				]
			}
		}]
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":2,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		},
		{
			"_id": 2,
			"_score":1480,
			"_source":
			{
				"content":"Text 2",
				"name":"Doc 2",
				"cat":2
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 2</b>"
				]
			}
		}]
	}
}
```


<!-- end -->


## 通过SQL进行高亮

`HIGHLIGHT()`函数可用于高亮搜索结果。语法如下：

```sql
HIGHLIGHT([options], [field_list], [query] )
```

<!-- example highlight() no args -->
默认情况下，它无需参数即可工作。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT() FROM books WHERE MATCH('before');
```

<!-- response SQL -->
```sql
+-----------------------------------------------------------+
| highlight()                                               |
+-----------------------------------------------------------+
| A door opened <b>before</b> them, revealing a small room. |
+-----------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() field syntax -->

`HIGHLIGHT()`从文档存储中检索所有可用的全文字段，并根据提供的查询对它们进行高亮。查询中的字段语法受支持。字段文本由`field_separator`分隔，该分隔符可在选项中修改。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT() FROM books WHERE MATCH('@title one');
```

<!-- response SQL -->
```sql
+-----------------+
| highlight()     |
+-----------------+
| Book <b>one</b> |
+-----------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() options -->
`HIGHLIGHT()`中的可选第一个参数是选项列表。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT({before_match='[match]',after_match='[/match]'}) FROM books WHERE MATCH('@title one');
```

<!-- response SQL -->
```sql
+------------------------------------------------------------+
| highlight({before_match='[match]',after_match='[/match]'}) |
+------------------------------------------------------------+
| Book [match]one[/match]                                    |
+------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() field list -->

可选的第二个参数是一个包含单个字段或逗号分隔字段列表的字符串。如果存在此参数，则仅从文档存储中获取并高亮指定的字段。空字符串作为第二个参数表示“获取所有可用字段”。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT({},'title,content') FROM books WHERE MATCH('one|robots');
```

<!-- response SQL -->
```sql
+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| highlight({},'title,content')                                                                                                                                                         |
+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Book <b>one</b> | They followed Bander. The <b>robots</b> remained at a polite distance, but their presence was a constantly felt threat.                                             |
| Bander ushered all three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander gestured the other <b>robots</b> away and entered itself. The door closed behind it. |
+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() string attr -->

或者，您可以使用第二个参数指定不带引号的字符串属性或字段名称。在这种情况下，提供的字符串将根据给定的查询进行高亮，但字段语法将被忽略。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT({}, title) FROM books WHERE MATCH('one');
```

<!-- response SQL -->
```sql
+---------------------+
| highlight({},title) |
+---------------------+
| Book <b>one</b>     |
| Book five           |
+---------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() query -->

可选的第三个参数是查询。这用于根据与搜索所用查询不同的查询来高亮搜索结果。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT({},'title', 'five') FROM books WHERE MATCH('one');
```

<!-- response SQL -->
```sql
+-------------------------------+
| highlight({},'title', 'five') |
+-------------------------------+
| Book one                      |
| Book <b>five</b>              |
+-------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example HIGHLIGHT TO_STRING -->

尽管`HIGHLIGHT()`设计用于处理存储的全文字段和字符串属性，但它也可用于高亮任意文本。请注意，如果查询包含任何字段搜索运算符（例如`@title hello @body world`），在这种情况下，其字段部分将被忽略。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT({},TO_STRING('some text to highlight'), 'highlight') FROM books WHERE MATCH('@title one');
```

<!-- response SQL -->
```sql
+----------------------------------------------------------------+
| highlight({},TO_STRING('some text to highlight'), 'highlight') |
+----------------------------------------------------------------+
| some text to <b>highlight</b>                                  |
+----------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

有几个选项仅当生成单个字符串作为结果（而不是片段数组）时才相关。这仅适用于SQL `HIGHLIGHT()`函数：

#### snippet_separator
插入片段之间的字符串。默认为` ... `。
#### field_separator
插入字段之间的字符串。默认为`|`。


另一种高亮文本的方法是使用[CALL SNIPPETS](../Searching/Highlighting.md#CALL-SNIPPETS)语句。这基本上复制了`HIGHLIGHT()`的功能，但无法使用内置文档存储。不过，它可以从文件加载源文本。


## 通过HTTP进行高亮

<!-- example highlight in JSON -->

要通过HTTP在JSON查询中高亮全文搜索结果，字段内容必须存储在文档存储中（默认启用）。在示例中，全文字段`content`和`title`从文档存储中获取，并根据`query`子句中指定的查询进行高亮。

高亮的片段在`hits`数组的`highlight`属性中返回。

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "*": "one|robots" } },
  "highlight":
  {
    "fields": ["content"]
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
        "_id": 1,
        "_score": 2788,
        "_source": {
          "title": "Books one",
          "content": "They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "
        },
        "highlight": {
          "content": [
            "They followed Bander. The <b>robots</b> remained at a polite distance, ",
            " three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander",
            " gestured the other <b>robots</b> away and entered itself. The"
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
$index->setName('books');
$results = $index->search('one|robots')->highlight(['content'])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}

```

<!-- response PHP -->
```php
Document: 1
title : Books one
content : They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it.
Highlight for content:
- They followed Bander. The <b>robots</b> remained at a polite distance,
-  three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander
-  gestured the other <b>robots</b> away and entered itself. The

```
<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content"]}}))
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'They followed Bander. The <b>robots</b> remained at a polite distance, ',
                                                u' three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander',
                                                u' gestured the other <b>robots</b> away and entered itself. The']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content"]}}))
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'They followed Bander. The <b>robots</b> remained at a polite distance, ',
                                                u' three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander',
                                                u' gestured the other <b>robots</b> away and entered itself. The']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content"]}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"content":["They followed Bander. The <b>robots</b> remained at a polite distance, "," three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander"," gestured the other <b>robots</b> away and entered itself. The"]}}]}}
```

<!-- intro -->
##### Java：

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","one|robots");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
    put("fields",new String[] {"content"});
}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- response Java -->
```java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: null
        hits: [{_id=1, _score=2788, _source={title=Books one, content=They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. }, highlight={title=[Books <b>one</b>], content=[They followed Bander. The <b>robots</b> remained at a polite distance, ,  three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander,  gestured the other <b>robots</b> away and entered itself. The]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "one|robots");
var highlight = new Highlight();
highlight.Fieldnames = new List<string> {"content"};
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);
```

<!-- response C# -->
```clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: null
        hits: [{_id=1, _score=2788, _source={title=Books one, content=They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. }, highlight={title=[Books <b>one</b>], content=[They followed Bander. The <b>robots</b> remained at a polite distance, ,  three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander,  gestured the other <b>robots</b> away and entered itself. The]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "one|robots".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};
let highlight_fields [String; 1] = ["content".to_string()];
let highlight = Highlight {
    fields: Some(serde_json::json!(highlight_fields)),
    ..Default::default(),
};

let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
```

<!-- response Rust -->
```rust
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: null
        hits: [{_id=1, _score=2788, _source={title=Books one, content=They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. }, highlight={title=[Books <b>one</b>], content=[They followed Bander. The <b>robots</b> remained at a polite distance, ,  three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander,  gestured the other <b>robots</b> away and entered itself. The]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'Text 1|Text 9'
    }
  },
  highlight: {}
});
```
<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1|Text 9"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example highlight JSON all field  -->

为了高亮所有可能的字段，请将空对象作为 `highlight` 属性传递。

<!-- intro -->
##### JSON：
<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "*": "one|robots" } },
  "highlight": {}
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
        "_id": 1,
        "_score": 2788,
        "_source": {
          "title": "Books one",
          "content": "They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "
        },
        "highlight": {
          "title": [
            "Books <b>one</b>"
          ],
          "content": [
            "They followed Bander. The <b>robots</b> remained at a polite distance, ",
            " three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander",
            " gestured the other <b>robots</b> away and entered itself. The"
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
##### PHP：

<!-- request PHP -->

```php
$index->setName('books');
$results = $index->search('one|robots')->highlight()->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}

```

<!-- response PHP -->
```php
Document: 1
title : Books one
content : They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it.
Highlight for title:
- Books <b>one</b>
Highlight for content:
- They followed Bander. The <b>robots</b> remained at a polite distance,
-  three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander
-  gestured the other <b>robots</b> away and entered itself. The

```
<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'They followed Bander. The <b>robots</b> remained at a polite distance, ',
                                                u' three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander',
                                                u' gestured the other <b>robots</b> away and entered itself. The'],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'They followed Bander. The <b>robots</b> remained at a polite distance, ',
                                                u' three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander',
                                                u' gestured the other <b>robots</b> away and entered itself. The'],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"title":["Books <b>one</b>"],"content":["They followed Bander. The <b>robots</b> remained at a polite distance, "," three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander"," gestured the other <b>robots</b> away and entered itself. The"]}}]}}

```

<!-- intro -->
##### Java：

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","one|robots");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- response Java -->
```java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: null
        hits: [{_id=1, _score=2788, _source={title=Books one, content=They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. }, highlight={title=[Books <b>one</b>], content=[They followed Bander. The <b>robots</b> remained at a polite distance, ,  three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander,  gestured the other <b>robots</b> away and entered itself. The]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "one|robots");
var highlight = new Highlight();
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);

```

<!-- response C# -->
```clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: null
        hits: [{_id=1, _score=2788, _source={title=Books one, content=They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. }, highlight={title=[Books <b>one</b>], content=[They followed Bander. The <b>robots</b> remained at a polite distance, ,  three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander,  gestured the other <b>robots</b> away and entered itself. The]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "one|robots".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};

let highlight = Highlight::new();
let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
```

<!-- response Rust -->
```rust
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: null
        hits: [{_id=1, _score=2788, _source={title=Books one, content=They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. }, highlight={title=[Books <b>one</b>], content=[They followed Bander. The <b>robots</b> remained at a polite distance, ,  three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander,  gestured the other <b>robots</b> away and entered itself. The]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'Text 1|Doc 1'
    }
  },
  highlight: {}
});
```
<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				],
				"name":
				[
					"<b>Doc 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1|Doc 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				],
				"name":
				[
					"<b>Doc 1</b>"
				]
			}
		]}
	}
}
```


<!-- end -->

除了常见的高亮选项外，还可以通过 HTTP 查询 JSON 查询时使用一些同义词：

#### fields
`fields` 对象包含属性名称及其选项。它也可以是一个字段名称的数组（没有选项）。

请注意，默认情况下，高亮尝试根据全文查询高亮结果。在一般情况下，如果不指定要高亮的字段，则高亮基于您的全文查询。但是，如果您指定了要高亮的字段，则仅在全文查询匹配选定字段时进行高亮。

#### encoder
`encoder` 可以设置为 `default` 或 `html`。设置为 `html` 时，高亮时保留 HTML 标记。这类似于 `html_strip_mode=retain` 选项。

<!-- example highlight_query -->
#### highlight_query
`highlight_query` 选项允许您使用除搜索查询之外的查询进行高亮。语法与主 `query` 语法相同。

<!-- intro -->
##### JSON：
<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "content": "one|robots" } },
  "highlight":
  {
    "fields": [ "content"],
    "highlight_query": { "match": { "*":"polite distance" } }
   }
}
```
<!-- intro -->
##### PHP：

<!-- request PHP -->

```php
$index->setName('books');
$bool = new \Manticoresearch\Query\BoolQuery();
$bool->must(new \Manticoresearch\Query\Match(['query' => 'one|robots'], 'content'));

$results = $index->search($bool)->highlight(['content'],['highlight_query'=>['match'=>['*'=>'polite distance']]])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```

<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"content":"one|robots"}},"highlight":{"fields":["content"],"highlight_query":{"match":{"*":"polite distance"}}}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'. The robots remained at a <b>polite distance</b>, but their presence was a']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"content":"one|robots"}},"highlight":{"fields":["content"],"highlight_query":{"match":{"*":"polite distance"}}}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'. The robots remained at a <b>polite distance</b>, but their presence was a']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"content":"one|robots"}},"highlight":{"fields":["content"],"highlight_query":{"match":{"*":"polite distance"}}}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1788,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"content":[". The robots remained at a <b>polite distance</b>, but their presence was a"]}}]}}
```


<!-- intro -->
##### Java：

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","one|robots");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
put("fields",new String[] {"content","title"});
put("highlight_query",
    new HashMap<String,Object>(){{
        put("match", new HashMap<String,Object>(){{
            put("*","polite distance");
        }});
    }});
}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "one|robots");
var highlight = new Highlight();
highlight.Fieldnames = new List<string> {"content", "title"};
Dictionary<string, Object> match = new Dictionary<string, Object>();
match.Add("*", "polite distance");
Dictionary<string, Object> highlightQuery = new Dictionary<string, Object>();
highlightQuery.Add("match", match);
highlight.HighlightQuery = highlightQuery;
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "one|robots".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};
let mut highlight_match_filter = HashMap::new();
highlight_match_filter.insert("*".to_string(), "polite distance".to_string());
let highlight_query = QueryFilter {
    r#match: Some(serde_json::json!(highlight_match_filter)),
    ..Default::default(),
};
let highlight_fields [String; 2] = ["content".to_string(), "title".to_string()];
let highlight = Highlight {
    fields: Some(serde_json::json!(highlight_fields)),
    highlight_query: Some(Box::new(highlight_query)),
    ..Default::default(),
};

let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'Text 1'
    }
  },
  highlight: {
    fields: ['content'],
    highlight_query: {
      match: {*: 'Text'}
    }
  }
});
```
<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text</b> 1"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlightField := manticoreclient.NetHighlightField("content")
highlightFields := []interface{} { highlightField }
highlight.SetFields(highlightFields)
queryMatchClause := map[string]interface{} {"*": "Text"};
highlightQuery := map[string]interface{} {"match": queryMatchClause};
highlight.SetHighlightQuery(highlightQuery)
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text</b> 1"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example pre_tags  -->

#### pre_tags 和 post_tags
`pre_tags` 和 `post_tags` 设置高亮文本片段的开闭标签。它们的功能类似于 `before_match` 和 `after_match` 选项。这些是可选的，默认值为 `<b>` 和 `</b>`。

<!-- intro -->
##### JSON：
<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "*": "one|robots" } },
  "highlight":
  {
    "fields": [ "content", "title" ],
    "pre_tags": "before_",
    "post_tags": "_after"
   }
}
```
<!-- intro -->
##### PHP：

<!-- request PHP -->

```php
$index->setName('books');
$bool = new \Manticoresearch\Query\BoolQuery();
$bool->must(new \Manticoresearch\Query\Match(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['pre_tags'=>'before_','post_tags'=>'_after'])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```
<!-- response PHP -->
```php
Document: 1
title : Books one
content : They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it.
Highlight for content:
- They followed Bander. The before_robots_after remained at a polite distance,
-  three into the room. before_One_after of the before_robots_after followed as well. Bander
-  gestured the other before_robots_after away and entered itself. The
Highlight for title:
- Books before_one_after

```
<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"pre_tags":"before_","post_tags":"_after"}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'They followed Bander. The before_robots_after remained at a polite distance, ',
                                                u' three into the room. before_One_after of the before_robots_after followed as well. Bander',
                                                u' gestured the other before_robots_after away and entered itself. The'],
                                   u'title': [u'Books before_one_after']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"pre_tags":"before_","post_tags":"_after"}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'They followed Bander. The before_robots_after remained at a polite distance, ',
                                                u' three into the room. before_One_after of the before_robots_after followed as well. Bander',
                                                u' gestured the other before_robots_after away and entered itself. The'],
                                   u'title': [u'Books before_one_after']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"pre_tags":"before_","post_tags":"_after"}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"content":["They followed Bander. The before_robots_after remained at a polite distance, "," three into the room. before_One_after of the before_robots_after followed as well. Bander"," gestured the other before_robots_after away and entered itself. The"],"title":["Books before_one_after"]}}]}}
```


<!-- intro -->
##### Java：

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","one|robots");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
    put("fields",new String[] {"content","title"});
    put("pre_tags","before_");
    put("post_tags","_after");
}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "one|robots");
var highlight = new Highlight();
highlight.Fieldnames = new List<string> {"content", "title"};
highlight.PreTags = "before_";
highlight.PostTags = "_after";
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "one|robots".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};
let highlight_fields [String; 2] = ["content".to_string(), "title".to_string()];
let highlight = Highlight {
    fields: Some(serde_json::json!(highlight_fields)),
    pre_tags: Some("before_".to_string()),
    post_tags: Some("_after".to_string()),
    ..Default::default(),
};

let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'Text 1'
    }
  },
  highlight: {
    pre_tags: 'before_',
    post_tags: '_after'
  }
});
```
<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"before_Text 1_after"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1"}
query := map[string]interface{} {"match": matchClause}
searchRequest.SetQuery(query)
highlight := manticoreclient.NewHighlight()
highlight.SetPreTags("before_")
highlight.SetPostTags("_after")
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"before_Text 1_after"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example no_match_size  -->
#### no_match_size
`no_match_size` 与 `allow_empty` 选项类似。如果设置为 0，则作为高亮结果返回空字符串，当无法生成片段时。否则，返回字段的开头。这是可选的，默认值为 1。

<!-- intro -->
##### JSON：
<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "*": "one|robots" } },
  "highlight":
  {
    "fields": [ "content", "title" ],
    "no_match_size": 0
  }
}
```
<!-- intro -->
##### PHP：

<!-- request PHP -->

```php
$index->setName('books');
$bool = new \Manticoresearch\Query\BoolQuery();
$bool->must(new \Manticoresearch\Query\Match(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['no_match_size'=>0])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```
<!-- response PHP -->
```php
Document: 1
title : Books one
content : They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it.
Highlight for content:
- They followed Bander. The <b>robots</b> remained at a polite distance,
-  three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander
-  gestured the other <b>robots</b> away and entered itself. The
Highlight for title:
- Books <b>one</b>
```
<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"no_match_size":0}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'They followed Bander. The <b>robots</b> remained at a polite distance, ',
                                                u' three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander',
                                                u' gestured the other <b>robots</b> away and entered itself. The'],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"no_match_size":0}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'They followed Bander. The <b>robots</b> remained at a polite distance, ',
                                                u' three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander',
                                                u' gestured the other <b>robots</b> away and entered itself. The'],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"no_match_size":0}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"content":["They followed Bander. The <b>robots</b> remained at a polite distance, "," three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander"," gestured the other <b>robots</b> away and entered itself. The"],"title":["Books <b>one</b>"]}}]}}

```

<!-- intro -->
##### Java：

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","one|robots");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
    put("fields",new String[] {"content","title"});
    put("no_match_size",0);
}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "one|robots");
var highlight = new Highlight();
highlight.Fieldnames = new List<string> {"content", "title"};
highlight.NoMatchSize = 0;
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "one|robots".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};
let highlight_fields [String; 2] = ["content".to_string(), "title".to_string()];
let highlight = Highlight {
    fields: Some(serde_json::json!(highlight_fields)),
    no_match_size: Some(NoMatchSize::Variant0),
    ..Default::default(),
};

let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'Text 1'
    }
  },
  highlight: {no_match_size: 0}
});
```
<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlight.SetNoMatchSize(0)
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example order  -->
#### order
`order` 设置提取片段的排序顺序。如果设置为 `"score"`，则按相关性顺序排序提取的片段。这是可选的，类似于 `weight_order` 选项。

<!-- intro -->
##### JSON：
<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "*": "one|robots" } },
  "highlight":
  {
    "fields": [ "content", "title" ],
    "order": "score"
  }
}
```

<!-- request PHP -->

```php
$index->setName('books');
$bool = new \Manticoresearch\Query\BoolQuery();
$bool->must(new \Manticoresearch\Query\Match(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['order'=>"score"])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```
<!-- response PHP -->
```php
Document: 1
title : Books one
content : They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it.
Highlight for content:
-  three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander
-  gestured the other <b>robots</b> away and entered itself. The
- They followed Bander. The <b>robots</b> remained at a polite distance,
Highlight for title:
- Books <b>one</b>
```
<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"order":"score"}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u' three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander',
                                                u' gestured the other <b>robots</b> away and entered itself. The',
                                                u'They followed Bander. The <b>robots</b> remained at a polite distance, '],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"order":"score"}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u' three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander',
                                                u' gestured the other <b>robots</b> away and entered itself. The',
                                                u'They followed Bander. The <b>robots</b> remained at a polite distance, '],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"order":"score"}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"content":[" three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander"," gestured the other <b>robots</b> away and entered itself. The","They followed Bander. The <b>robots</b> remained at a polite distance, "],"title":["Books <b>one</b>"]}}]}}


```
<!-- intro -->
##### Java：

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","one|robots");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
    put("fields",new String[] {"content","title"});
    put("order","score");
}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "one|robots");
var highlight = new Highlight();
highlight.Fieldnames = new List<string> {"content", "title"};
highlight.Order =  "score";
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "one|robots".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};
let highlight_fields [String; 2] = ["content".to_string(), "title".to_string()];
let highlight = Highlight {
    fields: Some(serde_json::json!(highlight_fields)),
    order: Some(Order::Score),
    post_tags: Some("_after".to_string()),
    ..Default::default(),
};

let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'Text 1'
    }
  },
  highlight: { order: 'score' }
});
```
<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlight.SetOrder("score")
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		]}
	}
}
```


<!-- end -->

<!-- example fragment_size -->
#### fragment_size
`fragment_size` 设置片段的最大符号数。它可以是全局的或字段特定的。字段特定选项会覆盖全局选项。这是可选的，默认值为 256。它类似于 `limit` 选项。

<!-- intro -->
##### JSON：
<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "*": "one|robots" } },
  "highlight":
  {
    "fields": [ "content", "title" ],
    "fragment_size": 100
  }
}
```
<!-- request PHP -->

```php
$index->setName('books');
$bool = new \Manticoresearch\Query\BoolQuery();
$bool->must(new \Manticoresearch\Query\Match(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['fragment_size'=>100])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```
<!-- response PHP -->
```php
Document: 1
title : Books one
content : They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it.
Highlight for content:
-  the room. <b>One</b> of the <b>robots</b> followed as well
- Bander gestured the other <b>robots</b> away and entered
Highlight for title:
- Books <b>one</b>
```
<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"fragment_size":100}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u' the room. <b>One</b> of the <b>robots</b> followed as well',
                                                u'Bander gestured the other <b>robots</b> away and entered '],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"fragment_size":100}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u' the room. <b>One</b> of the <b>robots</b> followed as well',
                                                u'Bander gestured the other <b>robots</b> away and entered '],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"fragment_size":100}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"content":[" the room. <b>One</b> of the <b>robots</b> followed as well","Bander gestured the other <b>robots</b> away and entered "],"title":["Books <b>one</b>"]}}]}}
```
<!-- intro -->
##### Java：

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","one|robots");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
    put("fields",new String[] {"content","title"});
    put("fragment_size",100);
}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "one|robots");
var highlight = new Highlight();
highlight.Fieldnames = new List<string> {"content", "title"};
highlight.FragmentSize = 100;
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "one|robots".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};
let highlight_fields [String; 2] = ["content".to_string(), "title".to_string()];
let highlight = Highlight {
    fields: Some(serde_json::json!(highlight_fields)),
    fragment_size: Some(serde_json::json!(100)),
    ..Default::default(),
};

let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'Text 1'
    }
  },
  highlight: { fragment_size: 4}
});
```
<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlight.SetFragmentSize(4)
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example number_of_fragments -->
#### number_of_fragments
`number_of_fragments` 限制结果中的最大片段数量。与 `fragment_size` 类似，它可以是全局的或字段特定的。这是可选的，默认值为 0（无限制）。它类似于 `limit_snippets` 选项。

<!-- intro -->
##### JSON：
<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "*": "one|robots"  } },
  "highlight":
  {
    "fields": [ "content", "title" ],
    "number_of_fragments": 10
  }
}
```
<!-- request PHP -->

```php
$index->setName('books');
$bool = new \Manticoresearch\Query\BoolQuery();
$bool->must(new \Manticoresearch\Query\Match(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['number_of_fragments'=>10])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```
<!-- response PHP -->
```php
Document: 1
title : Books one
content : They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it.
Highlight for content:
- They followed Bander. The <b>robots</b> remained at a polite distance,
-  three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander
-  gestured the other <b>robots</b> away and entered itself. The
Highlight for title:
- Books <b>one</b>

```

<!-- request Python -->
``` python
res =searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"number_of_fragments":10}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'They followed Bander. The <b>robots</b> remained at a polite distance, ',
                                                u' three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander',
                                                u' gestured the other <b>robots</b> away and entered itself. The'],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"number_of_fragments":10}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'They followed Bander. The <b>robots</b> remained at a polite distance, ',
                                                u' three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander',
                                                u' gestured the other <b>robots</b> away and entered itself. The'],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"number_of_fragments":10}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"content":["They followed Bander. The <b>robots</b> remained at a polite distance, "," three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander"," gestured the other <b>robots</b> away and entered itself. The"],"title":["Books <b>one</b>"]}}]}}

```
<!-- intro -->
##### Java：

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","one|robots");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
    put("fields",new String[] {"content","title"});
    put("number_of_fragments",10);
}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "one|robots");
var highlight = new Highlight();
highlight.Fieldnames = new List<string> {"content", "title"};
highlight.NumberOfFragments = 10;
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "one|robots".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};
let highlight_fields [String; 2] = ["content".to_string(), "title".to_string()];
let highlight = Highlight {
    fields: Some(serde_json::json!(highlight_fields)),
    number_of_fragments: Some(serde_json::json!(10)),
    ..Default::default(),
};

let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'Text 1'
    }
  },
  highlight: { number_of_fragments: 1}
});
```
<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlight.SetNumberOfFragments(1)
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example highlight json per-field limits -->

#### limit, limit_words, limit_snippets
选项如 `limit`、`limit_words` 和 `limit_snippets` 可以设置为全局或字段特定选项。全局选项会在字段特定选项未被覆盖时使用。在示例中，`title` 字段使用默认限制设置进行高亮显示，而 `content` 字段使用不同的限制。

<!-- intro -->
##### JSON：
<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "*": "one|robots"  } },
      "highlight":
      {
		"fields":
		{
			"title": {},
			"content" : { "limit": 50 }
		}
      }
}
```
<!-- request PHP -->

```php
$index->setName('books');
$bool = new \Manticoresearch\Query\BoolQuery();
$bool->must(new \Manticoresearch\Query\Match(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content'=>['limit'=>50],'title'=>new \stdClass])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```
<!-- response PHP -->
```php
Document: 1
title : Books one
content : They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it.
Highlight for content:
-  into the room. <b>One</b> of the <b>robots</b> followed as well
Highlight for title:
- Books <b>one</b>
```
<!-- request Python -->
``` python
res =searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":{"title":{},"content":{"limit":50}}}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u' into the room. <b>One</b> of the <b>robots</b> followed as well'],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":{"title":{},"content":{"limit":50}}}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u' into the room. <b>One</b> of the <b>robots</b> followed as well'],
                                   u'title': [u'Books <b>one</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":{"title":{},"content":{"limit":50}}}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"title":["Books <b>one</b>"],"content":[" into the room. <b>One</b> of the <b>robots</b> followed as well"]}}]}}
```
<!-- intro -->
##### Java：

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","one|robots");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
    put("fields",new HashMap<String,Object>(){{
            put("title",new HashMap<String,Object>(){{}});
            put("content",new HashMap<String,Object>(){{
                put("limit",50);
            }});
        }}
    );
}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "one|robots");
var highlight = new Highlight();
var highlightField = new HighlightField("title");
highlightField.Limit = 50;
highlight.Fields = new List<Object> {highlightField};
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "one|robots".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};
let highlight_fields [String; 1] = ["title".to_string()];
let highlight = Highlight {
    fields: Some(serde_json::json!(highlight_fields)),
    limit: Some(serde_json::json!(50)),
    ..Default::default(),
};

let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'Text 1'
    }
  },
  highlight: {
    fields: {
      content: { limit:1 }
    }
  }
});
```
<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlightField := manticoreclient.NetHighlightField("content")
highlightField.SetLimit(1);
highlightFields := []interface{} { highlightField }
highlight.SetFields(highlightFields)
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"hits":
		[{
			"_id": 1,
			"_score":1480,
			"_source":
			{
				"content":"Text 1",
				"name":"Doc 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Text</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example highlight json global limits -->

#### limits_per_field
也可以通过设置 `limits_per_field=0` 来强制执行全局限制。设置此选项意味着所有组合的高亮结果必须在指定的限制内。缺点是，如果高亮引擎认为某些内容更相关，则可能会在一个字段中获得多个片段高亮显示而在另一个字段中没有任何片段高亮显示。

<!-- intro -->
##### JSON：
<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "content": "and first" } },
      "highlight":
      {
        "limits_per_field": false,
		"fields":
		{
			"content" : { "limit": 50 }
		}
      }
}
```
<!-- request PHP -->

```php
$index->setName('books');
$bool = new \Manticoresearch\Query\BoolQuery();
$bool->must(new \Manticoresearch\Query\Match(['query' => 'and first'], 'content'));

$results = $index->search($bool)->highlight(['content'=>['limit'=>50]],['limits_per_field'=>false])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```
<!-- response PHP -->
```php
Document: 1
title : Books one
content : They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it.
Highlight for content:
-  gestured the other robots away <b>and</b> entered itself. The door closed
```
<!-- request Python -->
``` python
res =searchApi.search({"table":"books","query":{"match":{"content":"and first"}},"highlight":{"fields":{"content":{"limit":50}},"limits_per_field":False}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1597,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u' gestured the other robots away <b>and</b> entered itself. The door closed']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"content":"and first"}},"highlight":{"fields":{"content":{"limit":50}},"limits_per_field":False}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1597,
                    u'_source': {u'content': u'They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u' gestured the other robots away <b>and</b> entered itself. The door closed']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"content":"and first"}},"highlight":{"fields":{"content":{"limit":50}},"limits_per_field":false}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1597,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"content":[" gestured the other robots away <b>and</b> entered itself. The door closed"]}}]}}

```
<!-- intro -->
##### Java：

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","one|robots");
}});
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
    put("limits_per_field",0);
    put("fields",new HashMap<String,Object>(){{
            put("content",new HashMap<String,Object>(){{
                put("limit",50);
            }});
        }}
    );
}};
searchRequest.setHighlight(highlight);
searchResponse = searchApi.search(searchRequest);
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
var searchRequest = new SearchRequest("books");
searchRequest.FulltextFilter = new MatchFilter("*", "one|robots");
var highlight = new Highlight();
highlight.LimitsPerField = 0;
var highlightField = new HighlightField("title");
highlight.Fields = new List<Object> {highlightField};
searchRequest.Highlight = highlight;
var searchResponse = searchApi.Search(searchRequest);
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
let match_filter = HashMap::new();
match_filter.insert("*".to_string(), "one|robots".to_string());
let query = SearchQuery {
    match: Some(serde_json::json!(match_filter).into()),
    ..Default::default(),
};
let highlight_fields [String; 1] = ["title".to_string()];
let highlight = Highlight {
    fields: Some(serde_json::json!(highlight_fields)),
    limit_per_field: Some(serde_json::json!(false)),
    ..Default::default(),
};

let search_req = SearchRequest {
    table: "books".to_string(),
    query: Some(Box::new(query)),
    highlight: serde_json::json!(highlight),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'Text 1'
    }
  },
  highlight: { limits_per_field: 0 }
});
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Text 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlight.SetLimitsPerField(0)
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- end -->

## CALL SNIPPETS

<!-- example CALL SNIPPETS -->

`CALL SNIPPETS` 语句使用提供的数据和查询以及指定的表设置构建一个片段。它不能访问内置文档存储，因此建议使用 [HIGHLIGHT() 函数](../Searching/Highlighting.md)。

语法如下：

```sql
CALL SNIPPETS(data, table, query[, opt_value AS opt_name[, ...]])
```

#### data
`data` 是从中提取片段的来源。它可以是一个字符串或用花括号包围的字符串列表。
#### table
`table` 是提供片段生成文本处理设置的表名。
#### query
`query` 是用于构建片段的全文查询。
#### opt_value 和 opt_name
`opt_value` 和 `opt_name` 表示 [片段生成选项](../Searching/Highlighting.md)。

<!-- intro -->
##### SQL：
<!-- request SQL -->

```sql
CALL SNIPPETS(('this is my document text','this is my another text'), 'forum', 'is text', 5 AS around, 200 AS limit);
```
<!-- response SQL -->
```sql
+----------------------------------------+
| snippet                                |
+----------------------------------------+
| this <b>is</b> my document <b>text</b> |
| this <b>is</b> my another <b>text</b>  |
+----------------------------------------+
2 rows in set (0.02 sec)
```

<!-- end -->

大多数选项与[HIGHLIGHT()函数](../Searching/Highlighting.md)中的相同。然而，有几个选项只能与`CALL SNIPPETS`一起使用。

<!-- example CALL SNIPPETS load files -->
以下选项可用于突出显示存储在单独文件中的文本：

#### load_files
启用此选项时，将第一个参数视为文件名，而不是用于提取片段的数据。服务器端指定的文件将被加载作为数据。当启用此标志时，每个请求将使用最多[ max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)个工作线程来并行处理工作。默认值为0（无上限）。要在远程代理之间分发片段生成，请在仅包含一个本地代理和若干远程代理的分布式表中调用片段生成。[snippets_file_prefix](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#snippets_file_prefix)选项用于生成最终文件名。例如，当searchd配置为`snippets_file_prefix = /var/data_`且提供`text.txt`作为文件名时，片段将从`/var/data_text.txt`的内容中生成。

#### load_files_scattered
此选项仅适用于带有远程代理的分布式片段生成。片段生成的源文件可以分布在不同的代理上，主服务器将合并所有无错误的结果。例如，如果分布式表的一个代理拥有`file1.txt`，另一个代理拥有`file2.txt`，并且你使用包含这两个文件的`CALL SNIPPETS`，searchd将合并代理结果，因此你将获得来自`file1.txt`和`file2.txt`的结果。默认值为0。

如果同时启用了`load_files`选项，当任何文件在任何位置不可用时，请求将返回错误。否则（如果未启用`load_files`），所有缺失文件将返回空字符串。searchd不会将此标志传递给代理，因此如果文件不存在，代理不会生成严重错误。如果您想确保所有源文件都已加载，请将`load_files_scattered`和`load_files`都设置为1。如果某些代理缺少部分源文件不重要，则只需将`load_files_scattered`设置为1。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CALL SNIPPETS(('data/doc1.txt','data/doc2.txt'), 'forum', 'is text', 1 AS load_files);
```

<!-- response SQL -->
```
+----------------------------------------+
| snippet                                |
+----------------------------------------+
| this <b>is</b> my document <b>text</b> |
| this <b>is</b> my another <b>text</b>  |
+----------------------------------------+
2 rows in set (0.02 sec)
```

<!-- end -->
<!-- proofread -->

