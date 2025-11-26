# 高亮显示

<!-- example highlighting -->

高亮显示使您能够从包含匹配关键字的文档中获取高亮的文本片段（称为摘要）。

SQL 中的 `HIGHLIGHT()` 函数、通过 HTTP 的 JSON 查询中的 `"highlight"` 属性以及 PHP 客户端中的 `highlight()` 函数都利用内置的文档存储来检索原始字段内容（默认启用）。

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

使用 SQL 进行高亮搜索结果时，由于 MySQL 协议的限制，您将收到来自多个字段的摘要合并成的单个字符串。您可以通过下面详细说明的 `field_separator` 和 `snippet_separator` 选项调整连接分隔符。

通过 HTTP 执行 JSON 查询或使用 PHP 客户端时，没有此类限制，结果集包含字段数组，每个字段包含摘要数组（无分隔符）。

请注意，摘要生成选项如 `limit`、`limit_words` 和 `limit_snippets` 默认分别应用于每个字段。您可以使用 `limits_per_field` 选项更改此行为，但可能导致不希望的结果。例如，一个字段可能包含匹配关键字，但由于该字段的摘要排名不如其他字段，结果集中不包含该字段的摘要。

高亮算法当前优先考虑更好的摘要（短语匹配更接近），然后是包含尚未包含关键字的摘要。通常，它旨在突出显示查询的最佳匹配，并尽可能突出显示所有查询关键字（受限于限制）。如果当前字段中未找到匹配项，则默认根据限制修剪文档开头并返回。若要返回空字符串，请将 `allow_empty` 选项设置为 1。

高亮是在所谓的“后限制”阶段执行的，这意味着摘要生成不仅推迟到整个最终结果集准备好之后，还推迟到应用 LIMIT 子句之后。例如，对于 LIMIT 20,10 子句，`HIGHLIGHT()` 函数最多调用 10 次。

## 高亮选项

<!-- example highlighting options -->

有几个可选的高亮选项可用于微调摘要生成，这些选项在 SQL、HTTP 和 PHP 客户端中通用。

#### before_match
在关键字匹配前插入的字符串。此字符串中可以使用 `%SNIPPET_ID%` 宏。宏的第一次出现会被替换为当前摘要内递增的摘要编号。编号默认从 1 开始，但可以通过 `start_snippet_id` 选项覆盖。%SNIPPET_ID% 在每个新文档开始时重置。默认值为 `<b>`。

#### after_match
在关键字匹配后插入的字符串。默认值为 `</b>`。

#### limit
摘要的最大大小，单位为符号（代码点）。默认值为 256。默认情况下此限制按字段应用，详见 `limits_per_field`。

#### limit_words
限制结果中包含的最大单词数。请注意，此限制适用于所有单词，而不仅仅是要高亮的匹配关键字。例如，如果高亮 `Mary`，且选中的摘要为 `Mary had a little lamb`，则此摘要贡献了 5 个单词，而不仅仅是 1 个。默认值为 0（无限制）。默认情况下此限制按字段应用，详见 `limits_per_field`。

#### limit_snippets
限制结果中包含的最大摘要数。默认值为 0（无限制）。默认情况下此限制按字段应用，详见 `limits_per_field`。

#### limits_per_field
确定 `limit`、`limit_words` 和 `limit_snippets` 是作为每个被高亮文档字段的单独限制，还是作为整个文档的全局限制。将此选项设置为 0 表示一个文档的所有合并高亮结果必须在指定限制内。缺点是如果高亮引擎认为某些字段的摘要更相关，可能会在一个字段中高亮多个摘要，而另一个字段中没有摘要。默认值为 1（使用每字段限制）。
#### around
在每个匹配关键字块周围选择的单词数。默认值为 5。

#### use_boundaries
确定是否额外按短语边界字符拆分摘要，这些边界字符在表设置中通过 [phrase_boundary](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary) 指令配置。默认值为 0（不使用边界）。

#### weight_order
指定是否按相关性（权重递减）排序提取的摘要，或按文档中出现顺序（位置递增）排序。默认值为 0（不使用权重排序）。

#### force_all_words
忽略长度限制，直到结果包含所有关键字。默认值为 0（不强制包含所有关键字）。

#### start_snippet_id
设置 `%SNIPPET_ID%` 宏的起始值（该宏在 `before_match`、`after_match` 字符串中被检测并展开）。默认值为 1。

#### html_strip_mode
定义HTML剥离模式设置。默认值为`index`，表示将使用表设置。其他值包括`none`和`strip`，无论表设置如何，强制跳过或应用剥离；以及`retain`，保留HTML标记并保护其不被高亮。`retain`模式只能在高亮完整文档时使用，因此要求不设置片段大小限制。允许的字符串值为`none`、`strip`、`index`和`retain`。

#### allow_empty
允许在当前字段无法生成片段时（无关键字匹配或无片段符合限制）返回空字符串作为高亮结果。默认情况下，将返回原始文本的开头而不是空字符串。默认值为0（不允许空结果）。

#### snippet_boundary
确保片段不会跨越句子、段落或区域边界（当与启用了相应索引设置的表一起使用时）。允许的值为`sentence`、`paragraph`和`zone`。

#### emit_zones
在每个片段前发出包含区域名称的HTML标签。默认值为0（不发出区域名称）。

#### force_snippets
确定是否强制生成片段，即使限制允许高亮整个文本。默认值为0（不强制生成片段）。

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
默认情况下，无需参数即可使用。

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

`HIGHLIGHT()`从文档存储中检索所有可用的全文字段，并针对提供的查询进行高亮。查询中支持字段语法。字段文本由`field_separator`分隔，可在选项中修改。

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
`HIGHLIGHT()`的可选第一个参数是选项列表。

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

可选的第二个参数是包含单个字段或逗号分隔字段列表的字符串。如果存在此参数，则仅从文档存储中获取并高亮指定字段。第二个参数为空字符串表示“获取所有可用字段”。

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

或者，可以使用第二个参数指定字符串属性或字段名（不带引号）。在这种情况下，提供的字符串将针对查询进行高亮，但字段语法将被忽略。

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

可选的第三个参数是查询。用于针对与搜索时不同的查询进行高亮。

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

虽然`HIGHLIGHT()`设计用于存储的全文字段和字符串属性，但也可用于高亮任意文本。请注意，如果查询包含任何字段搜索操作符（例如`@title hello @body world`），则在此情况下忽略它们的字段部分。

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

某些选项仅在生成单个字符串结果（而非片段数组）时相关。这仅适用于SQL的`HIGHLIGHT()`函数：

#### snippet_separator
插入片段之间的字符串。默认值为` ... `。
#### field_separator
插入字段之间的字符串。默认值为`|`。


另一种高亮文本的方法是使用[CALL SNIPPETS](../Searching/Highlighting.md#CALL-SNIPPETS)语句。它大致复制了`HIGHLIGHT()`的功能，但不能使用内置文档存储。不过，它可以从文件加载源文本。


## 通过HTTP进行高亮

<!-- example highlight in JSON -->

要通过HTTP在JSON查询中高亮全文搜索结果，字段内容必须存储在文档存储中（默认启用）。示例中，从文档存储中获取全文字段`content`和`title`，并针对`query`子句中指定的查询进行高亮。

高亮片段在`hits`数组的`highlight`属性中返回。

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
##### Java:

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
##### C#:

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
##### Rust:

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

要突出显示所有可能的字段，请将一个空对象作为 `highlight` 属性传递。

<!-- intro -->
##### JSON:
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
##### PHP:

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
##### Java:

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
##### C#:

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
##### Rust:

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

除了常见的高亮选项外，通过 HTTP 的 JSON 查询还提供了几个同义词：

#### fields
`fields` 对象包含带有选项的属性名称。它也可以是字段名称的数组（不带任何选项）。

请注意，默认情况下，高亮尝试突出显示全文查询后的结果。在一般情况下，当您不指定要高亮的字段时，高亮基于您的全文查询。然而，如果您指定了要高亮的字段，则只有当全文查询匹配所选字段时才会高亮。

#### encoder
`encoder` 可以设置为 `default` 或 `html`。当设置为 `html` 时，它在高亮时保留 HTML 标记。这与 `html_strip_mode=retain` 选项的作用类似。

<!-- example highlight_query -->
#### highlight_query
`highlight_query` 选项允许您针对除搜索查询之外的查询进行高亮。语法与主 `query` 中相同。

<!-- intro -->
##### JSON:
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
##### PHP:

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
##### Java:

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
##### C#:

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
##### Rust:

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
`pre_tags` 和 `post_tags` 设置高亮文本片段的起始和结束标签。它们的功能类似于 `before_match` 和 `after_match` 选项。这些是可选的，默认值分别为 `<b>` 和 `</b>`。

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
    "fields": [ "content", "title" ],
    "pre_tags": "before_",
    "post_tags": "_after"
   }
}
```
<!-- intro -->
##### PHP:

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
##### Java:

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
##### C#:

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
##### Rust:

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
`no_match_size` 的功能类似于 `allow_empty` 选项。如果设置为 0，则相当于 `allow_empty=1`，允许在无法生成片段时返回空字符串作为高亮结果。否则，将返回字段的开头。这是可选的，默认值为 1。

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
    "fields": [ "content", "title" ],
    "no_match_size": 0
  }
}
```
<!-- intro -->
##### PHP:

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
##### Java:

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
##### C#:

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
##### Rust:

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
`order` 设置提取片段的排序顺序。如果设置为 `"score"`，则按相关性对提取的片段进行排序。此项为可选，且其工作方式类似于 `weight_order` 选项。

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
##### Java:

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
##### C#:

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
##### Rust:

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
`fragment_size` 设置片段的最大符号数。它可以是全局设置，也可以是按字段设置。按字段的选项会覆盖全局选项。此项为可选，默认值为 256。其工作方式类似于 `limit` 选项。

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
##### Java:

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
##### C#:

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
##### Rust:

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
`number_of_fragments` 限制结果中片段的最大数量。与 `fragment_size` 类似，它可以是全局设置或按字段设置。此项为可选，默认值为 0（无限制）。其工作方式类似于 `limit_snippets` 选项。

<!-- intro -->
##### JSON:
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
##### Java:

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
##### C#:

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
##### Rust:

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
选项如 `limit`、`limit_words` 和 `limit_snippets` 可以设置为全局或按字段的选项。全局选项作为按字段的限制使用，除非被按字段选项覆盖。在示例中，`title` 字段使用默认限制设置进行高亮，而 `content` 字段使用不同的限制。

<!-- intro -->
##### JSON:
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
##### Java:

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
##### C#:

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
##### Rust:

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
全局限制也可以通过指定 `limits_per_field=0` 来强制执行。设置此选项意味着所有合并的高亮结果必须在指定的限制内。缺点是，如果高亮引擎认为某些片段更相关，可能会在一个字段中获得多个高亮片段，而另一个字段则没有。

<!-- intro -->
##### JSON:
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
##### Java:

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
##### C#:

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
##### Rust:

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

`CALL SNIPPETS` 语句使用指定的表设置，从提供的数据和查询中构建片段。它无法访问内置的文档存储，因此建议使用 [HIGHLIGHT() 函数](../Searching/Highlighting.md)。

语法如下：

```sql
CALL SNIPPETS(data, table, query[, opt_value AS opt_name[, ...]])
```

#### data
`data` 作为提取片段的来源。它可以是单个字符串，也可以是用花括号括起来的字符串列表。
#### table
`table` 指定提供文本处理设置以生成片段的表名。
#### query
`query` 是用于构建摘要的全文查询。
#### opt_value 和 opt_name
`opt_value` 和 `opt_name` 表示[摘要生成选项](../Searching/Highlighting.md)。

<!-- intro -->
##### SQL:
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

大多数选项与[HIGHLIGHT() 函数](../Searching/Highlighting.md)中的相同。然而，有几个选项只能与 `CALL SNIPPETS` 一起使用。

<!-- example CALL SNIPPETS load files -->
以下选项可用于突出显示存储在单独文件中的文本：

#### load_files
启用此选项时，将第一个参数视为文件名，而不是用于提取摘要的数据。服务器端将加载指定的文件以获取数据。当启用此标志时，每个请求将使用最多 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) 个工作线程来并行处理工作。默认值为 0（无限制）。要在远程代理之间分配摘要生成，请在仅包含一个(!)本地代理和多个远程代理的分布式表中调用摘要生成。[snippets_file_prefix](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#snippets_file_prefix) 选项用于生成最终文件名。例如，当 searchd 配置为 `snippets_file_prefix = /var/data_` 并且提供了文件名 `text.txt` 时，摘要将从 `/var/data_text.txt` 的内容生成。

#### load_files_scattered
此选项仅适用于带有远程代理的分布式摘要生成。摘要生成的源文件可以分布在不同的代理上，主服务器将合并所有无错误的结果。例如，如果分布式表的一个代理有 `file1.txt`，另一个代理有 `file2.txt`，并且你使用 `CALL SNIPPETS` 处理这两个文件，searchd 将合并代理结果，因此你将获得来自 `file1.txt` 和 `file2.txt` 的结果。默认值为 0。

如果同时启用了 `load_files` 选项，则如果任何文件在任何地方不可用，请求将返回错误。否则（如果未启用 `load_files`），对于所有缺失的文件将返回空字符串。searchd 不会将此标志传递给代理，因此如果文件不存在，代理不会生成严重错误。如果你想确保所有源文件都被加载，请将 `load_files_scattered` 和 `load_files` 都设置为 1。如果某些代理上缺少某些源文件不是关键问题，只需将 `load_files_scattered` 设置为 1。

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

