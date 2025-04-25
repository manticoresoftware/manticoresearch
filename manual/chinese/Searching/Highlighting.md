# 高亮显示

<!-- 示例高亮 -->

高亮显示使您能够从包含匹配关键字的文档中获取高亮文本片段（称为剪辑）。

SQL `HIGHLIGHT()` 函数、通过 HTTP 的 JSON 查询中的 `"highlight"` 属性和 PHP 客户端中的 `highlight()` 函数都利用内置文档存储来检索原始字段内容（默认启用）。

<!-- 简介 -->
##### SQL:

<!-- 请求 SQL -->

```sql
SELECT HIGHLIGHT() FROM books WHERE MATCH('try');
```

<!-- 响应 SQL -->

```sql
+----------------------------------------------------------+
| highlight()                                              |
+----------------------------------------------------------+
| Don`t <b>try</b> to compete in childishness, said Bliss. |
+----------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- 简介 -->
##### JSON:

<!-- 请求 JSON -->

```json
POST /search
{
  "table": "books",
  "query":  {  "match": { "*" : "try" }  },
  "highlight": {}
}
```

<!-- 响应 JSON -->

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

<!-- 简介 -->
##### PHP:

<!-- 请求 PHP -->

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
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
        }
    }
}

```

<!-- 响应 PHP -->
```php
Document: 14
title: Book four
content: Don`t try to compete in childishness, said Bliss.
Highlight for title:
- Book four
Highlight for content:
- Don`t <b>try</b> to compete in childishness, said Bliss.

```

<!-- 请求 Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"try"}},"highlight":{}})
```
<!-- 响应 Python -->
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

<!-- 请求 Python-asyncio -->
``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"try"}},"highlight":{}})
```
<!-- 响应 Python-asyncio -->
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

<!-- 请求 Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"try"}},"highlight":{}});
```
<!-- 响应 Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 4,"_score":1695,"_source":{"title":"Book four","content":"Don`t try to compete in childishness, said Bliss."},"highlight":{"title":["Book four"],"content":["Don`t <b>try</b> to compete in childishness, said Bliss."]}}]}}
```

<!-- 简介 -->
##### Java:

<!-- 请求 Java -->

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

<!-- 响应 Java -->
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

When using SQL for highlighting search results, you will receive snippets from various fields combined into a single string due to the limitations of the MySQL protocol. You can adjust the concatenation separators with the `field_separator` and `snippet_separator` options, as detailed below.

When executing JSON queries through HTTP or using the PHP client, there are no such constraints, and the result set includes an array of fields containing arrays of snippets (without separators).

Keep in mind that snippet generation options like `limit`, `limit_words`, and `limit_snippets` apply to each field individually by default. You can alter this behavior using the `limits_per_field` option, but it could lead to unwanted results. For example, one field may have matching keywords, but no snippets from that field are included in the result set because they didn't rank as high as snippets from other fields in the highlighting engine.

The highlighting algorithm currently prioritizes better snippets (with closer phrase matches) and then snippets with keywords not yet included in the result. Generally, it aims to highlight the best match for the query and to highlight all query keywords, as allowed by the limits. If no matches are found in the current field, the beginning of the document will be trimmed according to the limits and returned by default. To return an empty string instead, set the `allow_empty` option to 1.

Highlighting is performed during the so-called `post limit` stage, which means that snippet generation is deferred not only until the entire final result set is prepared but also after the LIMIT clause is applied. For instance, with a LIMIT 20,10 clause, the `HIGHLIGHT()` function will be called a maximum of 10 times.

## Highlighting options

<!-- example highlighting options -->
有几个可选的高亮选项可以用于微调片段生成，这些选项对 SQL、HTTP 和 PHP 客户端都是通用的。

#### before_match
在关键字匹配前插入的字符串。可以在此字符串中使用 `%SNIPPET_ID%` 宏。此宏的第一次出现被当前片段中的递增片段编号所替代。编号默认从 1 开始，但可以通过 `start_snippet_id` 选项重写。%SNIPPET_ID% 在每个新文档的开头重新开始。默认值是 `<b>`。

#### after_match
在关键字匹配后插入的字符串。默认值是 `</b>`。

#### limit
最大片段大小，以符号（代码点）为单位。默认值是 256。此设置默认按字段应用，请参见 `limits_per_field`。

#### limit_words
限制结果中可以包含的最大单词数。请注意，该限制适用于所有单词，而不仅仅是需要高亮的匹配关键字。例如，如果高亮 `Mary`，并选择片段 `Mary had a little lamb`，它对这个限制贡献 5 个单词，而不仅仅是 1。默认值是 0（没有限制）。此设置默认按字段应用，请参见 `limits_per_field`。

#### limit_snippets
限制可以包含在结果中的最大片段数。默认值是 0（没有限制）。此设置默认按字段应用，请参见 `limits_per_field`。

#### limits_per_field
确定 `limit`、`limit_words` 和 `limit_snippets` 是否在待高亮的文档的每个字段中作为独立限制操作，或者作为整个文档的全局限制。将此选项设置为 0 意味着一个文档的所有组合高亮结果必须在指定的限制内。缺点是，如果高亮引擎决定它们更相关，您可能在一个字段中有多个高亮片段而在另一个字段中没有。默认值是 1（使用按字段的限制）。
#### around
选择每个匹配关键字块周围的单词数量。默认值是 5。

#### use_boundaries
确定是否根据表设置中的短语边界字符额外拆分片段，配置使用 [phrase_boundary](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary) 指令。默认值是 0（不使用边界）。

#### weight_order
指定是否按照相关性（递减权重）或文档中的出现顺序（递增位置）对提取的片段进行排序。默认值是 0（不使用权重顺序）。

#### force_all_words
忽略长度限制，直到结果中包含所有关键字。默认值是 0（不强制所有关键字）。

#### start_snippet_id
设置 `%SNIPPET_ID%` 宏的起始值（在 `before_match`、`after_match` 字符串中检测并扩展）。默认值是 1。

#### html_strip_mode
定义 HTML 剥离模式设置。默认为 `index`，这意味着将使用表设置。其他值包括 `none` 和 `strip`，它们强制跳过或应用剥离，而不考虑表设置；以及 `retain`，它保留 HTML 标记并保护其不被高亮。`retain` 模式仅在高亮整个文档时可以使用，因此要求不设置片段大小限制。允许的字符串值为 `none`、`strip`、`index` 和 `retain`。

#### allow_empty
允许在当前字段中无法生成片段时返回空字符串作为高亮结果（没有关键字匹配或没有片段符合限制）。默认情况下，返回原始文本的开头，而不是空字符串。默认值是 0（不允许空结果）。

#### snippet_boundary
确保片段不跨越句子、段落或区域边界（当与具有相应索引设置的表一起使用时）。允许的值为 `sentence`、`paragraph` 和 `zone`。

#### emit_zones
在每个片段之前发出一个带有封闭区域名称的 HTML 标签。默认值是 0（不发出区域名称）。

#### force_snippets
确定是否强制生成片段，即使限制允许高亮整个文本。默认值是 0（不强制生成片段）。

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
          "content":"Trevize whispered, "It gets infantile pleasure out of display. I`d love to knock it down.""
        },
        "highlight":
        {
          "title":
          [
            "Book three"
          ],
          "content":
          [
            ", "It <b>gets</b> infantile pleasure ",
            " to knock it <b>down</b>.""
          ]
        }
      },
      {
        "_id": 4,
        "_score":1573,
        "_source":
        {
          "title":"书四",
          "content":"不要试图在幼稚方面竞争，布利斯说道。"
        },
        "highlight":
        {
          "title":
          [
            "书四"
          ],
          "content":
          [
            "不要 <b>试图</b> 在幼稚方面竞争，<b>布利斯</b>说道。"
          ]
        }
      },
      {
        "_id": 2,
        "_score":1521,
        "_source":
        {
          "title":"书二",
          "content":"一扇门在他们面前打开，显露出一个小房间。班德说，"来吧，半人类，我想给你们看看我们是如何生活的。""
        },
        "highlight":
        {
          "title":
          [
            "书二"
          ],
          "content":
          [
            "一个小房间。班德 <b>说道</b>，"来吧，半人类，我"
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
    echo '文档: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "关于".$field."的高亮:
";
        foreach($snippets as $snippet)
        {
            echo  $snippet."
";
        }
    }
}
```

<!-- response PHP -->
```php
文档: 3
title: 书三
content: 特雷维兹低声说道，"它从展示中获得幼稚的乐趣。我想把它推倒。"
关于title的高亮:
- 书四
关于content的高亮:
, "它 <b>获得</b> 幼稚的乐趣
把它 <b>推倒</b>。"

文档: 4
title: 书四
content: 不要试图在幼稚方面竞争，布利斯说道。
关于title的高亮:
- 书四
关于content的高亮:
不要 <b>试图</b> 在幼稚方面竞争，<b>布利斯</b>说道。

文档: 2
title: 书二
content: 一扇门在他们面前打开，显露出一个小房间。班德说，"来吧，半人类，我想给你们看看我们是如何生活的。
关于title的高亮:
- 书二
关于content的高亮:
 一个小房间。班德 <b>说道</b>，"来吧，半人类，我
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
                    u'_source': {u'content': u'不要试图在幼稚方面竞争，布利斯说道。',
                                 u'title': u'书四'},
                    u'highlight': {u'content': [u'不要 <b>试图</b> 在幼稚方面竞争，布利斯说道。'],
                                   u'title': [u'书四']}}],
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
                    u'_source': {u'content': u'不要试图在幼稚方面竞争，布利斯说道。',
                                 u'title': u'书四'},
                    u'highlight': {u'content': [u'不要 <b>试图</b> 在幼稚方面竞争，布利斯说道。'],
                                   u'title': [u'书四']}}],
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
{"took":0,"timed_out":false,"hits":{"total":3,"hits":[{"_id": 3,"_score":1597,"_source":{"title":"书三","content":"特雷维兹低声说道，"它从展示中获得幼稚的乐趣。我想把它推倒。""},"highlight":{"title":["书三"],"content":[", "它 <b>获得</b> 幼稚的乐趣 "," 把它 <b>推倒</b>。""]}},{"_id": 4,"_score":1563,"_source":{"title":"书四","content":"不要试图在幼稚方面竞争，布利斯说道。"},"highlight":{"title":["书四"],"content":["不要 <b>试图</b> 在幼稚方面竞争，<b>布利斯</b>说道。"]}},{"_id": 5,"_score":1514,"_source":{"title":"书二","content":"一扇门在他们面前打开，显露出一个小房间。班德说，"来吧，半人类，我想给你们看看我们是如何生活的。""},"highlight":{"title":["书二"],"content":[" 一个小房间。班德 <b>说道</b>，"来吧，半人类，我"]}}]}}
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
        hits: [{_id=3, _score=1597, _source={title=书籍三, content=特雷维兹低声说，“它从展示中获得幼稚的乐趣。我很想把它打倒。”}, highlight={title=[书籍三], content=[, "它 <b>获得</b> 幼稚的乐趣 ,  打倒它 <b>的</b>。"]}}, {_id=4, _score=1563, _source={title=书籍四, content=“不要试图在幼稚中竞争，”布利斯说。}, highlight={title=[书籍四], content=[“不要 <b>试图</b> 在幼稚中竞争，<b>布利斯</b> 说。"]}}, {_id=5, _score=1514, _source={title=书籍二, content=一扇门在他们面前打开，露出了一个小房间。班德说：“来吧，半人类，我想向你们展示我们是如何生活的。”}, highlight={title=[书籍二], content=[ 一个小房间。班德 <b>说</b>，“来吧，半人类，我]}}]
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
        hits: [{_id=3, _score=1597, _source={title=书籍三, content=特雷维兹低声说，“它从展示中获得幼稚的乐趣。我很想把它打倒。”}, highlight={title=[书籍三], content=[, "它 <b>获得</b> 幼稚的乐趣 ,  打倒它 <b>的</b>。"]}}, {_id=4, _score=1563, _source={title=书籍四, content=“不要试图在幼稚中竞争，”布利斯说。}, highlight={title=[书籍四], content=[“不要 <b>试图</b> 在幼稚中竞争，<b>布利斯</b> 说。"]}}, {_id=5, _score=1514, _source={title=书籍二, content=一扇门在他们面前打开，露出了一个小房间。班德说：“来吧，半人类，我想向你们展示我们是如何生活的。”}, highlight={title=[书籍二], content=[ 一个小房间。班德 <b>说</b>，“来吧，半人类，我]}}]
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
        hits: [{_id=3, _score=1597, _source={title=书籍三, content=特雷维兹低声说，“它从展示中获得幼稚的乐趣。我很想把它打倒。”}, highlight={title=[书籍三], content=[, "它 <b>获得</b> 幼稚的乐趣 ,  打倒它 <b>的</b>。"]}}, {_id=4, _score=1563, _source={title=书籍四, content=“不要试图在幼稚中竞争，”布利斯说。}, highlight={title=[书籍四], content=[“不要 <b>试图</b> 在幼稚中竞争，<b>布利斯</b> 说。"]}}, {_id=5, _score=1514, _source={title=书籍二, content=一扇门在他们面前打开，露出了一个小房间。班德说：“来吧，半人类，我想向你们展示我们是如何生活的。”}, highlight={title=[书籍二], content=[ 一个小房间。班德 <b>说</b>，“来吧，半人类，我]}}]
        aggregations: null
    }
    profile: null
}
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: { match: { *: '文本 } },
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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"<b>文本 1</b>"

]

}

},

{

"_id": 2,

"_score":1480,

"_source":

{

"content":"文本 2",

"name":"文档 2",

"cat":2

},

"highlight":

{

"content":

[

"<b>文本 2</b>"

]

}

}]

}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"};
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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"<b>文本 1</b>"

]

}

},

{

"_id": 2,

"_score":1480,

"_source":

{

"content":"文本 2",

"name":"文档 2",

"cat":2

},

"highlight":

{

"content":

[

"<b>文本 2</b>"

]

}

}]

}
}
```


<!-- end -->


## Highlighting via SQL

The `HIGHLIGHT()` function can be used to highlight search results. Here's the syntax:

```sql
HIGHLIGHT([options], [field_list], [query] )
```

<!-- example highlight() no args -->
By default, it works with no arguments.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT() FROM books WHERE MATCH('之前');
```

<!-- response SQL -->
```sql
+-----------------------------------------------------------+
| highlight()                                               |
+-----------------------------------------------------------+
| 一扇门在他们面前打开，透露出一个小房间。                             |
+-----------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() field syntax -->

`HIGHLIGHT()` 从文档存储中检索所有可用的全文字段并根据提供的查询进行高亮。查询中的字段语法是支持的。字段文本由 `field_separator` 分隔，可以在选项中进行修改。

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
| 书籍 <b>one</b> |
+-----------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() options -->
`HIGHLIGHT()` 的可选第一个参数是选项列表。

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
| 书籍 [match]one[/match]                                    |
+------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() field list -->

可选的第二个参数是一个包含单个字段或以逗号分隔的字段列表的字符串。如果此参数存在，则仅从文档存储中获取指定的字段并进行高亮。将空字符串作为第二个参数表示“获取所有可用字段。”

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
| 书籍 <b>one</b> | 他们跟着班德。<b>机器人</b>保持在一个礼貌的距离，但他们的存在是一个持续的威胁。                                   |
| 班德把所有三个人都引入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德朝其他<b>机器人</b>挥手，将它们驱赶开，并自己进入。门在它身后关上。 |
+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() string attr -->

或者，您可以使用第二个参数来指定一个字符串属性或字段名称，而不加引号。在这种情况下，提供的字符串将根据提供的查询进行高亮，但字段语法将被忽略。

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
| 书籍 <b>one</b>     |
| 书籍 five           |
+---------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() query -->

可选的第三个参数是查询。这用于根据与用于搜索的查询不同的查询高亮搜索结果。

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
| 书籍 one                      |
| 书籍 <b>five</b>              |
+-------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example HIGHLIGHT TO_STRING -->

尽管 `HIGHLIGHT()` 旨在与存储的全文字段和字符串属性一起使用，但它也可以用来高亮任意文本。请记住，如果查询包含任何字段搜索运算符（例如，`@title hello @body world`），则此情况下将忽略字段部分。

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
| 一些文本以 <b>高亮</b>                                        |
+----------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

生成单个字符串作为结果（而不是数组片段）时，一些选项是相关的。这仅适用于 SQL `HIGHLIGHT()` 函数：

#### snippet_separator
用于在片段之间插入的字符串。默认值为 ` ... `。
#### field_separator
在字段之间插入的字符串。 默认值是 `|`。


另一种突出显示文本的方法是使用 [CALL SNIPPETS](../Searching/Highlighting.md#CALL-SNIPPETS) 语句。 这大多重复了 `HIGHLIGHT()` 的功能，但无法使用内置文档存储。 但是，它可以从文件加载源文本。


## 通过 HTTP 突出显示

<!-- example highlight in JSON -->

要通过 HTTP 在 JSON 查询中突出显示全文搜索结果，字段内容必须存储在文档存储中（默认启用）。 在示例中，全文字段 `content` 和 `title` 从文档存储中获取，并根据 `query` 子句中指定的查询进行突出显示。

突出显示的片段在 `hits` 数组的 `highlight` 属性中返回。

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
          "title": "书籍一",
          "content": "他们跟随班德。 机器人保持在礼貌的距离，但他们的存在始终是一种潜在威胁。 班德把他们三个引入房间。 一台机器人也跟随而来。 班德示意其他机器人离开，自己进入。 门在它身后关闭。 "
        },
        "highlight": {
          "content": [
            "他们跟随班德。 <b>机器人</b>保持在礼貌的距离，",
            "三个进入房间。 <b>一台</b> <b>机器人</b>也跟随而来。 班德",
            "示意其他<b>机器人</b>离开，自己进入。"
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
    echo '文档: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "对 ".$field." 的突出显示:
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
        }
    }
}

```

<!-- response PHP -->
```php
文档: 1
title : 书籍一
content : 他们跟随班德。 机器人保持在礼貌的距离，但他们的存在始终是一种潜在威胁。 班德把他们三个引入房间。 一台机器人也跟随而来。 班德示意其他机器人离开，自己进入。 门在它身后关闭。
对 content 的突出显示:
- 他们跟随班德。 <b>机器人</b>保持在礼貌的距离，
- 进入房间。 <b>一台</b> <b>机器人</b>也跟随而来。 班德
- 示意其他 <b>机器人</b>离开，自己进入。 
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
                    u'_source': {u'content': u'他们跟随班德。 机器人保持在礼貌的距离，但他们的存在始终是一种潜在威胁。 班德把他们三个引入房间。 一台机器人也跟随而来。 班德示意其他机器人离开，自己进入。 门在它身后关闭。 ',
                                 u'title': u'书籍一'},
                    u'highlight': {u'content': [u'他们跟随班德。 <b>机器人</b>保持在礼貌的距离，',
                                                u' 进入房间。 <b>一台</b> <b>机器人</b>也跟随而来。 班德',
                                                u' 示意其他 <b>机器人</b>离开，自己进入。 ']}}}],
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
                    u'_source': {u'content': u'他们跟随班德。 机器人保持在礼貌的距离，但他们的存在始终是一种潜在威胁。 班德把他们三个引入房间。 一台机器人也跟随而来。 班德示意其他机器人离开，自己进入。 门在它身后关闭。 ',
                                 u'title': u'书籍一'},
                    u'highlight': {u'content': [u'他们跟随班德。 <b>机器人</b>保持在礼貌的距离，',
                                                u' 进入房间。 <b>一台</b> <b>机器人</b>也跟随而来。 班德',
                                                u' 示意其他 <b>机器人</b>离开，自己进入。 ']}}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍一","content":"他们跟随班德。机器人保持在礼貌的距离，但它们的存在始终是一种威胁。班德将三人引入房间。一个机器人也跟着进来。班德示意其他机器人远离，并自己走了进去。门在它身后关闭。"},"highlight":{"content":["他们跟随班德。这个<b>机器人</b>保持在礼貌的距离，","三人进入房间。<b>其中一个</b> <b>机器人</b>也跟着进来。班德","示意其他<b>机器人</b>远离并自己走了进去。"]}}]}}
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
        hits: [{_id=1, _score=2788, _source={title=书籍一, content=他们跟随班德。机器人保持在礼貌的距离，但它们的存在始终是一种威胁。班德将三人引入房间。一个机器人也跟着进来。班德示意其他机器人远离，并自己走了进去。门在它身后关闭。 }, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。这个 <b>机器人</b> 保持在礼貌的距离， ,  三人进入房间。<b>其中一个</b> <b>机器人</b> 也跟着进来。班德,  示意其他 <b>机器人</b> 远离并自己走了进去。]}]}
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
        hits: [{_id=1, _score=2788, _source={title=书籍一, content=他们跟随班德。机器人保持在礼貌的距离，但它们的存在始终是一种威胁。班德将三人引入房间。一个机器人也跟着进来。班德示意其他机器人远离，并自己走了进去。门在它身后关闭。 }, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。这个 <b>机器人</b> 保持在礼貌的距离， ,  三人进入房间。<b>其中一个</b> <b>机器人</b> 也跟着进来。班德,  示意其他 <b>机器人</b> 远离并自己走了进去。]}]}
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
        hits: [{_id=1, _score=2788, _source={title=书籍一, content=他们跟随班德。机器人保持在礼貌的距离，但它们的存在始终是一种威胁。班德将三人引入房间。一个机器人也跟着进来。班德示意其他机器人远离，并自己走了进去。门在它身后关闭。 }, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。这个 <b>机器人</b> 保持在礼貌的距离， ,  三人进入房间。<b>其中一个</b> <b>机器人</b> 也跟着进来。班德,  示意其他 <b>机器人</b> 远离并自己走了进去。]}]}
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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"<b>文本 1</b>"

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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"<b>文本 1</b>"

]

}

]}

}
}

```

<!-- end -->

<!-- 示例突出显示 JSON 所有字段  -->

要突出显示所有可能的字段，将一个空对象作为 `highlight` 属性传递。

<!-- intro -->
##### JSON:

<!-- 请求 JSON -->

```json
POST /search
{
"table": "books",
"query": { "match": { "*": "one|robots" } },
"highlight": {}
}
```

<!-- 响应 JSON -->

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
        "title": "书籍一",
        "content": "他们跟随班德。机器人保持在礼貌的距离，但它们的存在是一个不断感受到的威胁。班德把三个人都带进了房间。一个机器人也跟了进去。班德示意其他机器人离开，然后自己进入。门在它身后关上。"
      },
      "highlight": {
        "title": [
          "书籍 <b>一</b>"
        ],
        "content": [
          "他们跟随班德。<b>机器人</b>保持在礼貌的距离，",
          " 三个人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德",
          " 示意其他<b>机器人</b>离开，然后自己进入。"
        ]
      }
    }
  ]
}
}
```

<!-- intro -->
##### PHP:

<!-- 请求 PHP -->

```php
$index->setName('books');
$results = $index->search('one|robots')->highlight()->get();
foreach($results as $doc)
{
  echo '文档: '.$doc->getId()."
";
  foreach($doc->getData() as $field=>$value)
  {
      echo $field.' : '.$value."
";
  }
  foreach($doc->getHighlight() as $field=>$snippets)
  {
      echo "对 ".$field." 的突出显示:
";
      foreach($snippets as $snippet)
      {
          echo "- ".$snippet."
";
      }
  }
}
```

<!-- 响应 PHP -->

```php
文档: 1
title : 书籍一
content : 他们跟随班德。机器人保持在礼貌的距离，但它们的存在是一个不断感受到的威胁。班德把三个人都带进了房间。一个机器人也跟了进去。班德示意其他机器人离开，然后自己进入。门在它身后关上。
对 title 的突出显示:
- 书籍 <b>一</b>
对 content 的突出显示:
- 他们跟随班德。<b>机器人</b>保持在礼貌的距离，
-  三个人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德
-  示意其他<b>机器人</b>离开，然后自己进入。
```

<!-- 请求 Python -->

``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{}).
```

<!-- 响应 Python -->

``` python
{'aggregations': None,
hits': {'hits': [{u'_id': u'1',
                  u'_score': 2788,
                  u'_source': {u'content': u'他们跟随班德。机器人保持在礼貌的距离，但它们的存在是一个不断感受到的威胁。班德把三个人都带进了房间。一个机器人也跟了进去。班德示意其他机器人离开，然后自己进入。门在它身后关上。',
                               u'title': u'书籍一'},
                  u'highlight': {u'content': [u'他们跟随班德。<b>机器人</b>保持在礼貌的距离，',
                                              u' 三个人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德',
                                              u' 示意其他<b>机器人</b>离开，然后自己进入。'],
                                 u'title': [u'书籍 <b>一</b>']}}],
        'max_score': None,
        'total': 1},
profile': None,
timed_out': False,
took': 0}
```

<!-- 请求 Python-asyncio -->

``` python
res = await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{}}).
```

<!-- 响应 Python-asyncio -->

``` python
{'aggregations': None,
hits': {'hits': [{u'_id': u'1',
                  u'_score': 2788,
                  u'_source': {u'content': u'他们跟随班德。机器人保持在礼貌的距离，但它们的存在是一个不断感受到的威胁。班德把三个人都带进了房间。一个机器人也跟了进去。班德示意其他机器人离开，然后自己进入。门在它身后关上。',
                               u'title': u'书籍一'},
                  u'highlight': {u'content': [u'他们跟随班德。<b>机器人</b>保持在礼貌的距离，',
                                              u' 三个人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德',
                                              u' 示意其他<b>机器人</b>离开，然后自己进入。'],
                                 u'title': [u'书籍 <b>一</b>']}}],
        'max_score': None,
        'total': 1},
profile': None,
timed_out': False,
took': 0}
```

<!-- 请求 Javascript -->

``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{}}).
```

<!-- 响应 Javascript -->

``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍一","content":"他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在始终是一个潜在的威胁。班德把三个人带进了房间。一个机器人也跟了进来。班德示意其他机器人离开，然后自己也进去了。门在它身后关上了。 "},"highlight":{"title":["书籍 <b>一</b>"],"content":["他们跟随班德。<b>机器人</b>保持在一个礼貌的距离， "," 三个人进入房间。<b>一个</b> <b>机器人</b>也跟随进来。班德","示意其他<b>机器人</b>离开，然后自己也进去了。"]}}]}}

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
        hits: [{_id=1, _score=2788, _source={title=书籍一, content=他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在始终是一个潜在的威胁。班德把三个人带进了房间。一个机器人也跟了进来。班德示意其他机器人离开，然后自己也进去了。门在它身后关上了。 }, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。<b>机器人</b>保持在一个礼貌的距离， ,  三个人进入房间。<b>一个</b> <b>机器人</b>也跟随进来。班德,  示意其他<b>机器人</b>离开，然后自己也进去了。]}}]
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
        hits: [{_id=1, _score=2788, _source={title=书籍一, content=他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在始终是一个潜在的威胁。班德把三个人带进了房间。一个机器人也跟了进来。班德示意其他机器人离开，然后自己也进去了。门在它身后关上了。 }, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。<b>机器人</b>保持在一个礼貌的距离， ,  三个人进入房间。<b>一个</b> <b>机器人</b>也跟随进来。班德,  示意其他<b>机器人</b>离开，然后自己也进去了。]}}]
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
        hits: [{_id=1, _score=2788, _source={title=书籍一, content=他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在始终是一个潜在的威胁。班德把三个人带进了房间。一个机器人也跟了进来。班德示意其他机器人离开，然后自己也进去了。门在它身后关上了。 }, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。<b>机器人</b>保持在一个礼貌的距离， ,  三个人进入房间。<b>一个</b> <b>机器人</b>也跟随进来。班德,  示意其他<b>机器人</b>离开，然后自己也进去了。]}}]
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
      *: '文本 1|文档 1'
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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"<b>文本 1</b>"

],

"name":

[

"<b>文档 1</b>"

]

}

]}

}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1|文档 1"};
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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"<b>文本 1</b>"

],

"name":

[

"<b>文档 1</b>"

]

}

]}
}
}
```


<!-- end -->

除了常见的高亮选项外，多个同义词可以通过 HTTP 的 JSON 查询使用：

#### fields
`fields` 对象包含属性名称和选项。它也可以是字段名称的数组（没有任何选项）。

请注意，默认情况下，高亮尝试对全文查询后的结果进行高亮。在一般情况下，如果不指定要高亮的字段，高亮是基于您的全文查询。但是，如果您指定了要高亮的字段，则仅当全文查询与所选字段匹配时，才会进行高亮。

#### encoder
`encoder` 可以设置为 `default` 或 `html`。设置为 `html` 时，它在高亮时保留 HTML 标记。这与 `html_strip_mode=retain` 选项类似。

<!-- example highlight_query -->
#### highlight_query
`highlight_query` 选项允许您针对与您的搜索查询不同的查询进行高亮。语法与主 `query` 中相同。

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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], 'content'));

$results = $index->search($bool)->highlight(['content'],['highlight_query'=>['match'=>['*'=>'polite distance']]])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
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

#### pre_tags and post_tags
`pre_tags` and `post_tags` set the opening and closing tags for highlighted text snippets. They function similarly to the `before_match` and `after_match` options. These are optional, with default values of `<b>` and `</b>`.

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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['pre_tags'=>'before_','post_tags'=>'_after'])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
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
                    u'highlight': {u'content': [u'他们跟随班德。 之前_机器人_之后保持在礼貌的距离，',
                                                u'三个人走进房间。 之前_一个_之后的之前_机器人_之后也跟随。 班德',
                                                u'示意其他之前_机器人_之后离开，并且自己进入。 The'],
                                   u'title': [u'书籍 之前_一个_之后']}}],
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
                    u'_source': {u'content': u'他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一个感受到的威胁。班德把所有三个人引入房间。一个机器人也跟随。班德示意其他机器人离开，并自己进入。门在它身后关上。',
                                 u'title': u'书籍 一个'},
                    u'highlight': {u'content': [u'他们跟随班德。 之前_机器人_之后保持在礼貌的距离，',
                                                u'三个人走进房间。 之前_一个_之后的之前_机器人_之后也跟随。 班德',
                                                u'示意其他之前_机器人_之后离开，并且自己进入。 The'],
                                   u'title': [u'书籍 之前_一个_之后']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍 一个","content":"他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一个感受到的威胁。班德把所有三个人引入房间。一个机器人也跟随。班德示意其他机器人离开，并自己进入。门在它身后关上。 "},"highlight":{"content":["他们跟随班德。 之前_机器人_之后保持在礼貌的距离， ","三个人走进房间。 之前_一个_之后的之前_机器人_之后也跟随。 班德","示意其他之前_机器人_之后离开，并且自己进入。 The"],"title":["书籍 之前_一个_之后"]}}]}}
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
      *: '文本 1'
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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"before_文本 1_after"

]

}

]}

}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"}
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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"before_文本 1_after"

]

}

]}

}
}
```

<!-- end -->

<!-- example no_match_size  -->
#### no_match_size
`no_match_size` 与 `allow_empty` 选项类似。如果设置为 0，它的作用相当于 `allow_empty=1`，允许在无法生成片段时返回空字符串作为高亮结果。否则，将返回字段的开头。这是可选的，默认值为 1。

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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['no_match_size'=>0])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
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
`order` 设置提取的代码片段的排序顺序。如果设置为 `"score"`，则按相关性对提取的代码片段进行排序。这是可选的，并且与 `weight_order` 选项的工作方式类似。

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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['order'=>"score"])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
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
                                   u'title': [u'书籍 <b>一个</b>']}}],
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
                    u'_source': {u'content': u'他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一种感受到的威胁。班德将所有三人引入房间。一台机器人也跟随其后。班德示意其他机器人离开，并自行进入。门在它后面关闭。',
                                 u'title': u'书籍 一个'},
                    u'highlight': {u'content': [u' 三人进入房间。 <b>一个</b> <b>机器人</b>也跟随其后。班德',
                                                u' 示意其他 <b>机器人</b> 离开并自行进入。',
                                                u'他们跟随班德。<b>机器人</b>保持在礼貌的距离，'],
                                   u'title': [u'书籍 <b>一个</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍 一个","content":"他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一种感受到的威胁。班德将所有三人引入房间。一台机器人也跟随其后。班德示意其他机器人离开，并自行进入。门在它后面关闭。 "},"highlight":{"content":[" 三人进入房间。 <b>一个</b> <b>机器人</b>也跟随其后。班德"," 示意其他 <b>机器人</b> 离开并自行进入。","他们跟随班德。<b>机器人</b>保持在礼貌的距离，"],"title":["书籍 <b>一个</b>"]}}]}}


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
      *: '文本 1'
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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"<b>文本 1</b>"

]

}

]}

}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"};
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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"<b>文本 1</b>"

]

}

]}

}
}
```


<!-- end -->

<!-- example fragment_size -->
#### fragment_size
`fragment_size` 设置最大片段大小，以符号为单位。它可以是全局或每个字段的。每个字段的选项会覆盖全局选项。这是可选的，默认值为256。它的工作方式类似于 `limit` 选项。

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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['fragment_size'=>100])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
        }
    }
}
```
<!-- response PHP -->
```php
Document: 1
title : 书籍 一个
content : 他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一种感受到的威胁。班德把三个全部带入了房间。一个机器人也跟着进来了。班德示意其他机器人离开，并自己进入。门在它身后关上。
Highlight for content:
-  房间。 <b>一个</b> 机器人也跟着进来了
- 班德示意其他 <b>机器人</b> 离开并进入
Highlight for title:
- 书籍 <b>一个</b>
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
                    u'_source': {u'content': u'他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一种感受到的威胁。班德把三个全部带入了房间。一个机器人也跟着进来了。班德示意其他机器人离开，并自己进入。门在它身后关上。 ',
                                 u'title': u'书籍 一个'},
                    u'highlight': {u'content': [u' 房间。 <b>一个</b> 机器人也跟着进来了',
                                                u'班德示意其他 <b>机器人</b> 离开并进入 '],
                                   u'title': [u'书籍 <b>一个</b>']}}],
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
                    u'_source': {u'content': u'他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一种感受到的威胁。班德把三个全部带入了房间。一个机器人也跟着进来了。班德示意其他机器人离开，并自己进入。门在它身后关上。 ',
                                 u'title': u'书籍 一个'},
                    u'highlight': {u'content': [u' 房间。 <b>一个</b> 机器人也跟着进来了',
                                                u'班德示意其他 <b>机器人</b> 离开并进入 '],
                                   u'title': [u'书籍 <b>一个</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍 一个","content":"他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一种感受到的威胁。班德把三个全部带入了房间。一个机器人也跟着进来了。班德示意其他机器人离开，并自己进入。门在它身后关上。 "},"highlight":{"content":[" 房间。 <b>一个</b> 机器人也跟着进来了","班德示意其他 <b>机器人</b> 离开并进入 "],"title":["书籍 <b>一个</b>"]}}]}}
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
`number_of_fragments` limits the maximum number of snippets in the result. Like `fragment_size`, it can be global or per-field. This is optional, with a default value of 0 (no limit). It works similarly to the `limit_snippets` option.

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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['number_of_fragments'=>10])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
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
                                                u' 三个进入房间。 <b>一个</b> <b>机器人</b> 也跟随其后。 Bander',
                                                u' 让其他 <b>机器人</b> 离开，然后自己进入。 The'],
                                   u'title': [u'书籍 <b>一个</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍 一个","content":"他们跟随 Bander。 机器人保持在一个礼貌的距离，但他们的存在始终如一地感觉到威胁。 Bander 让所有三个进入房间。 一个机器人也跟随其后。 Bander 让其他机器人离开，然后自己进入。 门在它身后关上。 "},"highlight":{"content":["他们跟随 Bander。 <b>机器人</b> 保持在一个礼貌的距离，"," 三个进入房间。 <b>一个</b> <b>机器人</b> 也跟随其后。 Bander"," 让其他 <b>机器人</b> 离开，然后自己进入。 The"],"title":["书籍 <b>一个</b>"]}}]}}

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
      *: '文本 1'
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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"<b>文本 1</b>"

]

}

]}

}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"};
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

"content":"文本 1",

"name":"文档 1",

"cat":1

},

"highlight":

{

"content":

[

"<b>文本 1</b>"

]

}

]}

}
}
```

<!-- end -->

<!-- example highlight json per-field limits -->

#### limit, limit_words, limit_snippets
选项如 `limit`、`limit_words` 和 `limit_snippets` 可以设置为全局选项或每个字段的选项。 全局选项用作每个字段的限制，除非每个字段的选项重写它们。 在这个例子中，`title` 字段以默认限制设置进行高亮，而 `content` 字段使用不同的限制。

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

# 高亮显示

<!-- example highlighting -->

高亮显示使您能够从包含匹配关键词的文档中获取高亮文本片段（称为代码段）。

SQL `HIGHLIGHT()` 函数、通过 HTTP 的 JSON 查询中的 `"highlight"` 属性，以及 PHP 客户端中的 `highlight()` 函数都利用内置文档存储来检索原始字段内容（默认启用）。

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
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
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
        hits: [{_id=3, _score=1597, _source={title=第三本书, content=特雷维兹低语, "它从展示中获得幼稚快感。我真想将它打倒."}, highlight={title=[第三本书], content=[, "它 <b>从</b>展示中获得幼稚快感 ,  将其 <b>打倒</b>."]}}, {_id=4, _score=1563, _source={title=第四本书, content=布利斯说，不要试图在幼稚中竞争.}, highlight={title=[第四本书], content=[Don`t <b>试图</b>在幼稚中竞争, <b>说</b>布利斯.]}}, {_id=5, _score=1514, _source={title=书籍二, content=他们面前开了一扇门，露出一个小房间。Bander 说, "来吧，半人类，我想向你们展示我们的生活方式."}, highlight={title=[书籍二], content=[ 一个小房间。Bander <b>说</b>, "来吧，半人类, I]}}]
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

When using SQL for highlighting search results, you will receive snippets from various fields combined into a single string due to the limitations of the MySQL protocol. You can adjust the concatenation separators with the `field_separator` and `snippet_separator` options, as detailed below.

When executing JSON queries through HTTP or using the PHP client, there are no such constraints, and the result set includes an array of fields containing arrays of snippets (without separators).

Keep in mind that snippet generation options like `limit`, `limit_words`, and `limit_snippets` apply to each field individually by default. You can alter this behavior using the `limits_per_field` option, but it could lead to unwanted results. For example, one field may have matching keywords, but no snippets from that field are included in the result set because they didn't rank as high as snippets from other fields in the highlighting engine.

The highlighting algorithm currently prioritizes better snippets (with closer phrase matches) and then snippets with keywords not yet included in the result. Generally, it aims to highlight the best match for the query and to highlight all query keywords, as allowed by the limits. If no matches are found in the current field, the beginning of the document will be trimmed according to the limits and returned by default. To return an empty string instead, set the `allow_empty` option to 1.

Highlighting is performed during the so-called `post limit` stage, which means that snippet generation is deferred not only until the entire final result set is prepared but also after the LIMIT clause is applied. For instance, with a LIMIT 20,10 clause, the `HIGHLIGHT()` function will be called a maximum of 10 times.

## Highlighting options

<!-- example highlighting options -->

There are several optional highlighting options that can be used to fine-tune snippet generation, which are common to SQL, HTTP, and PHP clients.

#### before_match

A string to insert before a keyword match. The `%SNIPPET_ID%` macro can be used in this string. The first occurrence of the macro is replaced with an incrementing snippet number within the current snippet. Numbering starts at 1 by default but can be overridden with the `start_snippet_id` option. %SNIPPET_ID% restarts at the beginning of each new document. The default is `<b>`.

#### after_match

A string to insert after a keyword match. The default is `</b>`.

#### limit

The maximum snippet size, in symbols (codepoints). The default is 256. This is applied per-field by default, see `limits_per_field`.

#### limit_words

Limits the maximum number of words that can be included in the result. Note that this limit applies to all words, not just the matched keywords to highlight. For example, if highlighting `Mary` and a snippet `Mary had a little lamb` is selected, it contributes 5 words to this limit, not just 1. The default is 0 (no limit). This is applied per-field by default, see `limits_per_field`.

#### limit_snippets

Limits the maximum number of snippets that can be included in the result. The default is 0 (no limit). This is applied per-field by default, see `limits_per_field`.

#### limits_per_field

Determines whether `limit`, `limit_words`, and `limit_snippets` operate as individual limits in each field of the document being highlighted or as global limits for the entire document. Setting this option to 0 means that all combined highlighting results for one document must be within the specified limits. The downside is that you may have several snippets highlighted in one field and none in another if the highlighting engine decides they are more relevant. The default is 1 (use per-field limits).

#### around

The number of words to select around each matching keyword block. The default is 5.

#### use_boundaries
确定是否根据表设置中配置的[phrase_boundary](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary)指令额外按短语边界字符分割片段。默认值为0（不使用边界）。

#### weight_order
指定是按相关性（递减权重）对提取的片段进行排序，还是按文档中的出现顺序（递增位置）进行排序。默认值为0（不使用权重顺序）。

#### force_all_words
忽略长度限制，直到结果包含所有关键字。默认值为0（不强制使用所有关键字）。

#### start_snippet_id
设置`%SNIPPET_ID%`宏的起始值（该宏在`before_match`，`after_match`字符串中被检测和扩展）。默认值为1。

#### html_strip_mode
定义HTML剥离模式设置。默认为`index`，这意味着将使用表设置。其他值包括`none`和`strip`，会强制跳过或应用剥离，而不考虑表设置；以及`retain`，保留HTML标记并保护其不被高亮。`retain`模式只能在高亮整个文档时使用，因此要求不设置片段大小限制。允许的字符串值为`none`，`strip`，`index`和`retain`。

#### allow_empty
允许当当前字段无法生成片段时返回空字符串作为高亮结果（没有关键词匹配或没有片段符合限制）。默认情况下，原文本的开头将被返回，而不是空字符串。默认值为0（不允许空结果）。

#### snippet_boundary
确保片段不跨越句子、段落或区域边界（当与具有相应索引设置的表一起使用时）。允许的值为`sentence`、`paragraph`和`zone`。

#### emit_zones
在每个片段前发出一个包含区域名称的HTML标签。默认值为0（不发出区域名称）。

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
          "content":"Trevize whispered, "It gets infantile pleasure out of display. I`d love to knock it down.""
        },
        "highlight":
        {
          "title":
          [
            "Book three"
          ],
          "content":
          [
            ", "It <b>gets</b> infantile pleasure ",
            " to knock it <b>down</b>.""
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
          "content":"A door opened before them, revealing a small room. Bander said, "Come, half-humans, I want to show you how we live.""
        },
        "highlight":
        {
          "title":
          [
            "Book two"
          ],
          "content":
          [
            " a small room. Bander <b>said</b>, "Come, half-humans, I"
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
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo  $snippet."
";
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
title: 书二
content: 一扇门在他们面前打开，显露出一个小房间。班德说：“来吧，半人类，我想给你们看看我们是怎么生活的。”
Highlight for title:
- 书二
Highlight for content:
 一个小房间。班德 <b>说</b>，"来吧，半人类，我
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
                                 u'title': u'书四'},
                    u'highlight': {u'content': [u'Don`t <b>try</b> to compete in childishness, said Bliss.'],
                                   u'title': [u'书四']}}],
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
{"took":0,"timed_out":false,"hits":{"total":3,"hits":[{"_id": 3,"_score":1597,"_source":{"title":"书三","content":"特雷维兹低声说，"它从展示中获得幼稚的快感。我想把它打倒。""},"highlight":{"title":["书三"],"content":[", "它 <b>获得</b> 幼稚的快感 "," 想把它 <b>打倒</b>.""]}},{"_id": 4,"_score":1563,"_source":{"title":"书四","content":"Don`t try to compete in childishness, said Bliss."},"highlight":{"title":["书四"],"content":["Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss."]}},{"_id": 5,"_score":1514,"_source":{"title":"书二","content":"一扇门在他们面前打开，显露出一个小房间。班德说，"来吧，半人类，我想给你们看看我们是怎么生活的。""},"highlight":{"title":["书二"],"content":[" 一个小房间。班德 <b>说</b>，"来吧，半人类，我"]}}]}}
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
        hits: [{_id=3, _score=1597, _source={title=书三, content=特雷维兹低声说, "它从展示中获得幼稚的快感。我想把它打倒。"}, highlight={title=[书三], content=[, "它 <b>获得</b> 幼稚的快感 ,  想把它 <b>打倒</b>."]}}, {_id=4, _score=1563, _source={title=书四, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[书四], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=书二, content=一扇门在他们面前打开，显露出一个小房间。班德说，"来吧，半人类，我想给你们看看我们是怎么生活的。"}, highlight={title=[书二], content=[ 一个小房间。班德 <b>说</b>, "来吧，半人类，我]}}]
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
        hits: [{_id=3, _score=1597, _source={title=书三, content=特雷维兹低声说, "它从展示中获得幼稚的快感。我想把它打倒。"}, highlight={title=[书三], content=[, "它 <b>获得</b> 幼稚的快感 ,  想把它 <b>打倒</b>."]}}, {_id=4, _score=1563, _source={title=书四, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[书四], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=书二, content=一扇门在他们面前打开，显露出一个小房间。班德说，"来吧，半人类，我想给你们看看我们是怎么生活的。"}, highlight={title=[书二], content=[ 一个小房间。班德 <b>说</b>, "来吧，半人类，我]}}]
        aggregations: null
    }
    profile: null
}
```
<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: { match: { *: '文本' } },
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		},
		{
			"_id": 2,
			"_score":1480,
			"_source":
			{
				"content":"文本 2",
				"name":"文档 2",
				"cat":2
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 2</b>"
				]
			}
		}]
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"};
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		},
		{
			"_id": 2,
			"_score":1480,
			"_source":
			{
				"content":"文本 2",
				"name":"文档 2",
				"cat":2
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 2</b>"
				]
			}
		}]
	}
}
```


<!-- end -->


## 通过 SQL 高亮

`HIGHLIGHT()` 函数可用于高亮搜索结果。语法如下：

```sql
HIGHLIGHT([options], [field_list], [query] )
```

<!-- example highlight() no args -->
默认情况下，它不需要任何参数。

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
| 一扇门在他们面前打开，显露出一个小房间。                             |
+-----------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() field syntax -->

`HIGHLIGHT()` 从文档存储中检索所有可用的全文字段，并根据提供的查询对它们进行高亮。查询中的字段语法是支持的。字段文本以 `field_separator` 分隔，可以在选项中进行修改。

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
| 书籍 <b>one</b> |
+-----------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() options -->
`HIGHLIGHT()` 的可选第一个参数是选项列表。

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
| 书籍 [match]one[/match]                                    |
+------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() field list -->

可选的第二个参数是一个包含单个字段或以逗号分隔的字段列表的字符串。如果此参数存在，只会从文档存储中提取指定的字段并进行高亮。第二个参数为空字符串表示“提取所有可用字段”。

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
| 书籍 <b>one</b> | 他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一个感觉到的威胁。                                   |
| 班德把三个都引入房间。<b>一个</b>机器人也跟随进入。班德示意其他<b>机器人</b>离开，自己则进入。门在它身后关闭。 |
+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() string attr -->

另外，您可以使用第二个参数指定一个字符串属性或字段名而不加引号。在这种情况下，提供的字符串将在提供的查询中被高亮，但是字段语法将被忽略。

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
| 书籍 <b>one</b>     |
| 书籍五           |
+---------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() query -->

可选的第三个参数是查询。这用于对搜索结果进行高亮，使其与用于搜索的查询不同。

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
| 书籍 one                      |
| 书 <b>五</b>              |
+-------------------------------+
2 行在集合中 (0.00 秒)
```

<!-- end -->

<!-- example HIGHLIGHT TO_STRING -->

尽管 `HIGHLIGHT()` 旨在与存储的全文字段和字符串属性一起使用，但它也可以用来高亮任意文本。请注意，如果查询包含任何字段搜索操作符（例如 `@title hello @body world`），在这种情况下，它们的字段部分将被忽略。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT({},TO_STRING('一些要高亮的文本'), 'highlight') FROM books WHERE MATCH('@title one');
```

<!-- response SQL -->
```sql
+----------------------------------------------------------------+
| highlight({},TO_STRING('一些要高亮的文本'), 'highlight') |
+----------------------------------------------------------------+
| 一些要高亮的文本 <b>highlight</b>                                  |
+----------------------------------------------------------------+
1 行在集合中 (0.00 秒)
```

<!-- end -->

有几个选项仅在生成单个字符串作为结果时相关（而不是片段数组）。这仅适用于 SQL `HIGHLIGHT()` 函数：

#### snippet_separator
插入片段之间的字符串。默认值为 ` ... `。
#### field_separator
插入字段之间的字符串。默认值为 `|`。


另一种高亮文本的方法是使用 [CALL SNIPPETS](../Searching/Highlighting.md#CALL-SNIPPETS) 语句。它大部分复制了 `HIGHLIGHT()` 的功能，但不能使用内置的文档存储。然而，它可以从文件加载源文本。


## 通过 HTTP 高亮显示

<!-- example highlight in JSON -->

在通过 HTTP 的 JSON 查询中高亮显示全文搜索结果，字段内容必须存储在文档存储中（默认启用）。在此示例中，全文字段 `content` 和 `title` 从文档存储中获取，并根据 `query` 子句中指定的查询进行高亮显示。

高亮显示的片段在 `hits` 数组的 `highlight` 属性中返回。

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
          "title": "书籍一",
          "content": "他们跟着班德。机器人保持在一个礼貌的距离，但它们的存在始终是一种感觉到的威胁。班德把三个人都引进了房间。一个机器人也跟了进来。班德示意其他机器人离开，然后自己进去了。门在它身后关闭了。"
        },
        "highlight": {
          "content": [
            "他们跟着班德。<b>机器人</b>保持在一个礼貌的距离，",
            "三人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德",
            "示意其他<b>机器人</b>离开，然后自己进去了。"
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
    echo '文档: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "关于 ".$field." 的高亮:
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
        }
    }
}

```

<!-- response PHP -->
```php
文档: 1
title : 书籍一
content : 他们跟着班德。机器人保持在一个礼貌的距离，但它们的存在始终是一种感觉到的威胁。班德把三个人都引进了房间。一个机器人也跟了进来。班德示意其他机器人离开，然后自己进去了。门在它身后关闭了。
关于 content 的高亮:
- 他们跟着班德。<b>机器人</b>保持在一个礼貌的距离，
-  三人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德
-  示意其他<b>机器人</b>离开，然后自己进去了。  

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
                    u'_source': {u'content': u'他们跟着班德。机器人保持在一个礼貌的距离，但它们的存在始终是一种感觉到的威胁。班德把三个人都引进了房间。一个机器人也跟了进来。班德示意其他机器人离开，然后自己进去了。门在它身后关闭了。',
                                 u'title': u'书籍一'},
                    u'highlight': {u'content': [u'他们跟着班德。<b>机器人</b>保持在一个礼貌的距离，',
                                                u' 三人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德',
                                                u' 示意其他<b>机器人</b>离开，然后自己进去了。']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍 一","content":"他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在是一个不断感受到的威胁。班德把三人都引入房间。其中一个机器人也跟随而来。班德示意其他机器人离开，自己也进入了房间。门在后面关上了。"},"highlight":{"content":["他们跟随班德。这个<b>机器人</b>保持在一个礼貌的距离，","三人进了房间。<b>一个</b><b>机器人</b>也跟随而来。班德","示意其他<b>机器人</b>离开，自己也进入了房间。"]}}]}}
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
        hits: [{_id=1, _score=2788, _source={title=书籍 一, content=他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在是一个不断感受到的威胁。班德把三人都引入房间。其中一个机器人也跟随而来。班德示意其他机器人离开，自己也进入了房间。门在后面关上了。}, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。这个 <b>机器人</b>保持在一个礼貌的距离，,  三人进了房间。<b>一个</b><b>机器人</b>也跟随而来。班德,  示意其他<b>机器人</b>离开，自己也进入了房间。]}}]
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
        hits: [{_id=1, _score=2788, _source={title=书籍 一, content=他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在是一个不断感受到的威胁。班德把三人都引入房间。其中一个机器人也跟随而来。班德示意其他机器人离开，自己也进入了房间。门在后面关上了。}, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。这个 <b>机器人</b>保持在一个礼貌的距离，,  三人进了房间。<b>一个</b><b>机器人</b>也跟随而来。班德,  示意其他<b>机器人</b>离开，自己也进入了房间。]}}]
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
      *: '文本 1|文本 9'
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1|文本 9"};
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example highlight JSON all field  -->

To highlight all possible fields, pass an empty object as the `highlight` property.

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
          "title": "书籍 一",
          "content": "他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在是一个不断感受到的威胁。班德把三人都引入房间。其中一个机器人也跟随而来。班德示意其他机器人离开，自己也进入了房间。门在后面关上了。"
        },
        "highlight": {
          "title": [
            "书籍 <b>一</b>"
          ],
          "content": [
            "他们跟随班德。这个 <b>机器人</b>保持在一个礼貌的距离，",
            "三人进了房间。<b>一个</b><b>机器人</b>也跟随而来。班德",
            "示意其他 <b>机器人</b>离开，自己也进入了房间。"
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
    echo 'Document: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
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

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: '文本 1|文档 1'
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				],
				"name":
				[
					"<b>文档 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1|文档 1"};
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				],
				"name":
				[
					"<b>文档 1</b>"
				]
			}
		]}
	}
}
```


<!-- end -->

除了常见的高亮选项外，还有几个同义词可通过 HTTP 进行 JSON 查询：

#### fields
`fields` 对象包含带有选项的属性名称。它也可以是字段名称的数组（没有任何选项）。

请注意，默认情况下，高亮尝试高亮与全文查询匹配的结果。在一般情况下，当您不指定要高亮的字段时，高亮是基于您的全文查询的。然而，如果您指定要高亮的字段，只有在全文查询与所选字段匹配时才会高亮。

#### encoder
`encoder` 可以设置为 `default` 或 `html`。设置为 `html` 时，它在高亮时保留 HTML 标记。这与 `html_strip_mode=retain` 选项的工作方式相似。

<!-- example highlight_query -->
#### highlight_query
`highlight_query` 选项允许您针对与搜索查询不同的查询进行高亮。语法与主 `query` 中相同。

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
    "highlight_query": { "match": { "*":"礼貌距离" } }
   }
}
```
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('books');
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], 'content'));

$results = $index->search($bool)->highlight(['content'],['highlight_query'=>['match'=>['*'=>'礼貌距离']]])->get();
foreach($results as $doc)
{
    echo '文档: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "字段 ".$field." 的高亮:
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
        }
    }
}
```

<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"content":"one|robots"}},"highlight":{"fields":["content"],"highlight_query":{"match":{"*":"礼貌距离"}}}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1788,
                    u'_source': {u'content': u'他们跟随班德。机器人在礼貌距离内保持着，但他们的存在始终是一个不断感受到的威胁。班德把三个人都带进了房间。一个机器人也跟了进去。班德示意其他机器人离开，然后自己进入。门在它身后关上。 ',
                                 u'title': u'书籍之一'},
                    u'highlight': {u'content': [u'. 机器人保持在一个 <b>礼貌距离</b> 内，但他们的存在是一个']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"content":"one|robots"}},"highlight":{"fields":["content"],"highlight_query":{"match":{"*":"礼貌距离"}}}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1788,"_source":{"title":"书籍之一","content":"他们跟随班德。机器人在礼貌距离内保持着，但他们的存在始终是一个不断感受到的威胁。班德把三个人都带进了房间。一个机器人也跟了进去。班德示意其他机器人离开，然后自己进入。门在它身后关上。 "},"highlight":{"content":[". 机器人保持在一个 <b>礼貌距离</b> 内，但他们的存在是一个"]}}]}}
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
            put("*","礼貌距离");
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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['pre_tags'=>'before_','post_tags'=>'_after'])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
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
`no_match_size` 的功能类似于 `allow_empty` 选项。如果设置为 0，则表现为 `allow_empty=1`，在无法生成片段时允许返回空字符串作为高亮结果。否则，将返回该字段的开头部分。这是可选的，默认值为 1.

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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['no_match_size'=>0])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
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
                    u'_source': {u'content': u'他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一种不断感受到的威胁。班德招呼三人进入房间。另一个机器人也跟了进来。班德示意其他机器人远离，自己也进入了。门在它身后关上了。',
                                 u'title': u'书籍 一'},
                    u'highlight': {u'content': [u'他们跟随班德。这个 <b>机器人</b>保持在礼貌的距离，',
                                                u' 三人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德',
                                                u' 示意其他 <b>机器人</b>远离，自己也进入。'],
                                   u'title': [u'书籍 <b>一</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍 一","content":"他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一种不断感受到的威胁。班德招呼三人进入房间。另一个机器人也跟了进来。班德示意其他机器人远离，自己也进入了。门在它身后关上了。 "},"highlight":{"content":["他们跟随班德。这个 <b>机器人</b>保持在礼貌的距离， "," 三人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德"," 示意其他 <b>机器人</b>远离，自己也进入。"],"title":["书籍 <b>一</b>"]}}]}}

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

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: '文本 1'
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"};
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example order  -->
#### order
`order` sets the sorting order of extracted snippets. If set to `"score"`, it sorts the extracted snippets in order of relevance. This is optional and works similarly to the `weight_order` option.

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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['order'=>"score"])->get();
foreach($results as $doc)
{
    echo '文档: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "字段 ".$field." 的高亮:
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
        }
    }
}
```
<!-- response PHP -->
```php
文档: 1
title : 书籍 一
content : 他们跟随班德。机器人保持在礼貌的距离，但他们的存在始终是一种不断感受到的威胁。班德招呼三人进入房间。另一个机器人也跟了进来。班德示意其他机器人远离，自己也进入了。门在它身后关上了。
字段 content 的高亮:
-  三人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德
-  示意其他 <b>机器人</b> 离开并自己进入。该
-  他们跟随班德。 <b>机器人</b> 保持在礼貌的距离，
关注标题：
- 书籍 <b>一</b>
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
                    u'_source': {u'content': u'他们跟随班德。机器人保持在礼貌的距离，但他们的存在是一个持续的威胁。班德把所有三个人都带进了房间。一个机器人也跟了进来。班德示意其他机器人离开并自己进入。门在它身后关上。',
                                 u'title': u'书籍 一'},
                    u'highlight': {u'content': [u' 三个人进入房间。<b>一个</b> <b>机器人</b> 也跟了进来。班德',
                                                u' 示意其他 <b>机器人</b> 离开并自己进入。该',
                                                u'他们跟随班德。 <b>机器人</b> 保持在礼貌的距离，']},
                                   u'title': [u'书籍 <b>一</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍 一","content":"他们跟随班德。机器人保持在礼貌的距离，但他们的存在是一个持续的威胁。班德把所有三个人都带进了房间。一个机器人也跟了进来。班德示意其他机器人离开并自己进入。门在它身后关上。 "},"highlight":{"content":[" 三个人进入房间。<b>一个</b> <b>机器人</b> 也跟了进来。班德"," 示意其他 <b>机器人</b> 离开并自己进入。该","他们跟随班德。 <b>机器人</b> 保持在礼貌的距离， "],"title":["书籍 <b>一</b>"]}}]}}


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

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: '文本 1'
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"};
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```


<!-- end -->

<!-- example fragment_size -->
#### fragment_size
`fragment_size` 设置符号的最大片段大小。可以是全局或每字段的。每字段的选项会覆盖全局选项。这是可选的，默认值为 256。它的工作方式类似于 `limit` 选项。

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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['fragment_size'=>100])->get();
foreach($results as $doc)
{
    echo '文档: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "高亮 ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
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
`number_of_fragments` 限制结果中片段的最大数量。像 `fragment_size` 一样，它可以是全局或每字段的。这是可选的，默认值为 0（无限制）。它的工作方式与 `limit_snippets` 选项类似。

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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['number_of_fragments'=>10])->get();
foreach($results as $doc)
{
    echo 'Document: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Highlight for ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
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

                    u'highlight': {u'content': [u'They followed Bander. The <b>robots</b> remained at a polite distance,',

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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"content":["They followed Bander. The <b>robots</b> remained at a polite distance, "," three into the room. <b>One</b> of the <b>robots</b> followed as well. Bander"," gestured the other <b>robots</b> away and entered itself. The"],"title":["Books <b>one</b>"]}}]}

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
Options like `limit`, `limit_words`, and `limit_snippets` can be set as global or per-field options. Global options are used as per-field limits unless per-field options override them. In the example, the `title` field is highlighted with default limit settings, while the `content` field uses a different limit.
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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'one|robots'], '*'));
$results = $index->search($bool)->highlight(['content'=>['limit'=>50],'title'=>new stdClass])->get();
foreach($results as $doc)
{
  echo 'Document: '.$doc->getId()."
";
  foreach($doc->getData() as $field=>$value)
  {
      echo $field.' : '.$value."
";
  }
  foreach($doc->getHighlight() as $field=>$snippets)
  {
      echo "Highlight for ".$field.":
";
      foreach($snippets as $snippet)
      {
          echo "- ".$snippet."
";
      }
  }
}
```
<!-- response PHP -->
```php
Document: 1
title : 图书一
content : 他们跟随 Bander。机器人保持着礼貌的距离，但他们的存在是一个持续感受到的威胁。Bander 将三人都引导进房间。一个机器人也跟了进来。Bander 示意其他机器人离开，然后自己进入。门在它身后关上了。
Highlight for content:
- 进房间。<b>一个</b><b>机器人</b>也跟了进来
Highlight for title:
- 图书<b>一</b>
```
<!-- request Python -->
``` python
res =searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":{"title":{},"content":{"limit":50}}}})
```
<!-- response Python -->
``` python
{'aggregations': None,
hits': {'hits': [{u'_id': u'1',
                  u'_score': 2788,
                  u'_source': {u'content': u'他们跟随 Bander。机器人保持着礼貌的距离，但他们的存在是一个持续感受到的威胁。Bander 将三人都引导进房间。一个机器人也跟了进来。Bander 示意其他机器人离开，然后自己进入。门在它身后关上了。',
                               u'title': u'图书一'},
                  u'highlight': {u'content': [u' 进房间。<b>一个</b><b>机器人</b>也跟了进来'],
                                 u'title': [u'图书<b>一</b>']}}],
        'max_score': None,
        'total': 1},
profile': None,
timed_out': False,
took': 0}
```
<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":{"title":{},"content":{"limit":50}}}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"图书一","content":"他们跟随 Bander。机器人保持着礼貌的距离，但他们的存在是一个持续感受到的威胁。Bander 将三人都引导进房间。一个机器人也跟了进来。Bander 示意其他机器人离开，然后自己进入。门在它身后关上了。"},"highlight":{"title":["图书<b>一</b>"],"content":[" 进房间。<b>一个</b><b>机器人</b>也跟了进来"]}}]}}
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
took":0,
timed_out":false,
hits":

"total":1,
"hits":
[{
	"_id": 1,
	"_score":1480,
	"_source":
	{
		"content":"文本 1",
		"name":"文档 1", 
		"cat":1
	},
	"highlight":
	{
		"content":
		[
			"<b>文本</b>"
		]
	}
}]

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
took":0,
timed_out":false,
hits":

"total":1,
"hits":
[{
	"_id": 1,
	"_score":1480,
	"_source":
	{
		"content":"文本 1",
		"name":"文档 1",
		"cat":1
	},
	"highlight":
	{
				"content":
				[
					"<b>文本</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- 示例高亮 json 全局限制 -->

#### limits_per_field
全局限制也可以通过指定 `limits_per_field=0` 来强制实施。设置此选项意味着所有组合的高亮结果必须在指定的限制内。缺点是，如果高亮引擎决定它们更相关，您可能会在一个字段中获取多个高亮片段，而在另一个字段中没有。

<!-- 引言 -->
##### JSON:
<!-- 请求 JSON -->

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
<!-- 请求 PHP -->

```php
$index->setName('books');
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'and first'], 'content'));

$results = $index->search($bool)->highlight(['content'=>['limit'=>50]],['limits_per_field'=>false])->get();
foreach($results as $doc)
{
    echo '文档: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "高亮内容 ".$field.":
";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."
";
        }
    }
}
```
<!-- 响应 PHP -->
```php
Document: 1
title : Books one
content : They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it.
高亮内容 for content:
-  gestured the other robots away <b>and</b> entered itself. The door closed
```
<!-- 请求 Python -->
``` python
res =searchApi.search({"table":"books","query":{"match":{"content":"and first"}},"highlight":{"fields":{"content":{"limit":50}},"limits_per_field":False}})
```
<!-- 响应 Python -->
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

<!-- 请求 Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"content":"and first"}},"highlight":{"fields":{"content":{"limit":50}},"limits_per_field":false}});
```
<!-- 响应 Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1597,"_source":{"title":"Books one","content":"They followed Bander. The robots remained at a polite distance, but their presence was a constantly felt threat. Bander ushered all three into the room. One of the robots followed as well. Bander gestured the other robots away and entered itself. The door closed behind it. "},"highlight":{"content":[" gestured the other robots away <b>and</b> entered itself. The door closed"]}}]}}

```
<!-- 引言 -->
##### Java:

<!-- 请求 Java -->

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

<!-- 引言 -->
##### C#:

<!-- 请求 C# -->

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

<!-- 请求 TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: '文本 1'
    }
  },
  highlight: { limits_per_field: 0 }
});
```

<!-- 请求 Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlight.SetLimitsPerField(0)
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- end -->

# 调用片段

<!-- 示例 调用片段 -->

`CALL SNIPPETS` 语句根据提供的数据和查询使用指定的表设置构建片段。它无法访问内置文档存储，因此建议使用 [HIGHLIGHT() 函数](../Searching/Highlighting.md)。

语法为：

```sql
CALL SNIPPETS(data, table, query[, opt_value AS opt_name[, ...]])
```

#### data
`data` 作为提取片段的来源。它可以是单个字符串或用大括号括起来的字符串列表。
#### table
`table` 指提供片段生成的文本处理设置的表名。
#### query
`query` 是用于构建片段的全文查询。
#### opt_value 和 opt_name
`opt_value` 和 `opt_name` 代表 [片段生成选项](../Searching/Highlighting.md)。

<!-- intro -->
##### SQL:
<!-- request SQL -->

# 突出显示

<!-- example highlighting -->

突出显示使您能够从包含匹配关键字的文档中获取高亮文本片段（称为片段）。

SQL `HIGHLIGHT()` 函数，HTTP 中的 JSON 查询中的 `"highlight"` 属性，以及 PHP 客户端中的 `highlight()` 函数均利用内置文档存储来检索原始字段内容（默认启用）。

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
        hits: [{_id=3, _score=1597, _source={title=书三, content=特雷维兹低声说，“它从展示中获得幼稚的快乐。我想砸掉它。”}, highlight={title=[书三], content=[, "它 <b>获得</b> 幼稚的快乐 ,  想砸掉它 <b>。 </b>."]}}, {_id=4, _score=1563, _source={title=书四, content=布利斯说：“不要试图在幼稚上竞争。”}, highlight={title=[书四], content=[不要 <b>试图</b> 在幼稚上竞争, <b>布利斯说</b>。]}}, {_id=5, _score=1514, _source={title=书二, content=一扇门在他们面前打开，露出一个小房间。班德说：“来吧，半人类，我想给你们展示我们是如何生活的。”}, highlight={title=[书二], content=[ 一个小房间。班德 <b>说</b>，“来吧，半人类，我]}}]
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
      *: '文本 1'
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
				"content":"文本 1"
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"};
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
				"content":"文本 1"
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

使用 SQL 突出搜索结果时，由于 MySQL 协议的限制，您将收到来自各个字段的片段组合为一个字符串。您可以使用 `field_separator` 和 `snippet_separator` 选项调整连接分隔符，如下所述。

通过 HTTP 执行 JSON 查询或使用 PHP 客户端时，没有这样的限制，结果集包括包含片段数组的字段数组（没有分隔符）。

请记住，像 `limit`、`limit_words` 和 `limit_snippets` 这样的片段生成选项默认适用于每个字段。您可以使用 `limits_per_field` 选项更改此行为，但这可能导致不想要的结果。例如，一个字段可能有匹配的关键字，但该字段中没有片段包含在结果集中，因为它们在高亮引擎中没有像其他字段的片段那样获得高排名。

目前，高亮算法优先考虑更好的片段（与短语匹配更紧密），然后是尚未包含在结果中的关键字的片段。通常，它旨在突出查询的最佳匹配，并突出所有查询关键字，具体取决于限制。如果在当前字段中未找到匹配项，文档的开头将根据限制进行修剪并默认返回。要返回空字符串，请将 `allow_empty` 选项设置为 1。

高亮是在所谓的 `post limit` 阶段执行的，这意味着片段生成不仅被推迟到整个最终结果集准备好之后，而且还在应用 LIMIT 子句后。例如，使用 LIMIT 20,10 子句，`HIGHLIGHT()` 函数最多会被调用 10 次。

## 高亮选项

<!-- example highlighting options -->

有几种可选的高亮选项，可以用于微调片段生成，这些选项对于 SQL、HTTP 和 PHP 客户端是共同的。

#### before_match
在关键字匹配之前插入的字符串。可以在此字符串中使用 `%SNIPPET_ID%` 宏。宏的第一次出现会被替换为当前片段内的递增片段编号。编号默认从 1 开始，但可以使用 `start_snippet_id` 选项覆盖。%SNIPPET_ID% 在每个新文档的开头重新开始。默认值为 `<b>`。

#### after_match
在关键字匹配之后插入的字符串。默认值为 `</b>`。

#### limit
最大片段大小，以符号（代码点）为单位。默认为 256。默认情况下，按字段应用，见 `limits_per_field`。

#### limit_words
限制可以包含在结果中的最大单词数。请注意，此限制适用于所有单词，而不仅仅是要高亮的匹配关键字。例如，如果高亮 `玛丽` 并选择了片段 `玛丽有一只小羊`，则它对该限制贡献 5 个单词，而不仅仅是 1 个。默认值为 0（没有限制）。默认情况下，按字段应用，见 `limits_per_field`。

#### limit_snippets
限制可以包含在结果中的最大片段数。默认值为 0（没有限制）。默认情况下，按字段应用，见 `limits_per_field`。

#### limits_per_field
确定 `limit`、`limit_words` 和 `limit_snippets` 是否在被突出显示的文档的每个字段中作为独立限制操作，或作为整个文档的全局限制。将此选项设置为 0 意味着一个文档的所有组合高亮显示结果必须在指定限制内。缺点是，您可能会在一个字段中高亮显示多个片段，而在另一个字段中没有任何片段，如果高亮引擎决定它们更相关。默认值为 1（使用每个字段的限制）。
#### around
每个匹配关键字块周围选择的单词数。默认值为 5。

#### use_boundaries
确定是否按短语边界字符来额外拆分片段，具体配置在表设置中的 [phrase_boundary](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary) 指令。默认值为 0（不使用边界）。

#### weight_order
指定是否按相关性排序提取的片段（权重递减）或按文档中的出现顺序排序（位置递增）。默认值为 0（不使用权重排序）。

#### force_all_words
忽略长度限制，直到结果包含所有关键字。默认值为 0（不强制所有关键字）。

#### start_snippet_id
设置 `%SNIPPET_ID%` 宏的起始值（在 `before_match` 和 `after_match` 字符串中检测并扩展）。默认值为 1。

#### html_strip_mode
定义 HTML 去除模式设置。默认值为 `index`，意味着将使用表设置。其他值包括 `none` 和 `strip`，无论表设置如何，强制跳过或应用去除；以及 `retain`，保留 HTML 标记并保护其不被高亮。`retain` 模式仅在高亮完整文档时使用，因此要求不设置片段大小限制。允许的字符串值为 `none`、`strip`、`index` 和 `retain`。

#### allow_empty
允许当当前字段无法生成片段时返回空字符串作为高亮结果（无关键字匹配或无片段符合限制）。默认情况下，返回原始文本的开头而不是空字符串。默认值为 0（不允许空结果）。

#### snippet_boundary
确保片段不跨越句子、段落或区域边界（与启用了相应索引设置的表一起使用时）。允许的值为 `sentence`、`paragraph` 和 `zone`。

#### emit_zones
在每个片段之前发出包含区域名称的 HTML 标签。默认值为 0（不发出区域名称）。

#### force_snippets
确定是否强制生成片段，即使限制允许高亮整个文本。默认值为 0（不强制生成片段）。

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
title: 第二本书
content: 一扇门在他们面前打开，露出一个小房间。Bander 说：“来吧，半人类，我想给你们展示我们的生活。”
Highlight for title:
- 第二本书
Highlight for content:
 一个小房间。Bander <b>说</b>，“来吧，半人类，我
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
                                 u'title': u'第四本书'},
                    u'highlight': {u'content': [u'Don`t <b>try</b> to compete in childishness, said Bliss.'],
                                   u'title': [u'第四本书']}}],
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
{"took":0,"timed_out":false,"hits":{"total":3,"hits":[{"_id": 3,"_score":1597,"_source":{"title":"第三本书","content":"Trevize 低声说道，“它从展示中获得幼稚的快乐。我想把它摧毁。”"},"highlight":{"title":["第三本书"],"content":[", \"它 <b>获得</b> 幼稚的快乐 "," 把它 <b>摧毁</b>.\""]}},{"_id": 4,"_score":1563,"_source":{"title":"第四本书","content":"Don`t try to compete in childishness, said Bliss."},"highlight":{"title":["第四本书"],"content":["Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss."]}},{"_id": 5,"_score":1514,"_source":{"title":"第二本书","content":"一扇门在他们面前打开，露出一个小房间。Bander 说，“来吧，半人类，我想给你们展示我们的生活。”"},"highlight":{"title":["第二本书"],"content":[" 一个小房间。Bander <b>说</b>，“来吧，半人类，我"]}}]}}
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
        hits: [{_id=3, _score=1597, _source={title=第三本书, content=Trevize 低声说道, "它从展示中获得幼稚的快乐。我想把它摧毁。"}, highlight={title=[第三本书], content=[, "它 <b>获得</b> 幼稚的快乐 ,  把它 <b>摧毁</b>."]}}, {_id=4, _score=1563, _source={title=第四本书, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[第四本书], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=第二本书, content=一扇门在他们面前打开，露出一个小房间。Bander 说，“来吧，半人类，我想给你们展示我们的生活。”}, highlight={title=[第二本书], content=[ 一个小房间。Bander <b>说</b>，“来吧，半人类，我]}}]
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
        hits: [{_id=3, _score=1597, _source={title=第三本书, content=Trevize 低声说道, "它从展示中获得幼稚的快乐。我想把它摧毁。"}, highlight={title=[第三本书], content=[, "它 <b>获得</b> 幼稚的快乐 ,  把它 <b>摧毁</b>."]}}, {_id=4, _score=1563, _source={title=第四本书, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[第四本书], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=第二本书, content=一扇门在他们面前打开，露出一个小房间。Bander 说，“来吧，半人类，我想给你们展示我们的生活。”}, highlight={title=[第二本书], content=[ 一个小房间。Bander <b>说</b>，“来吧，半人类，我]}}]
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		},
		{
			"_id": 2,
			"_score":1480,
			"_source":
			{
				"content":"文本 2",
				"name":"文档 2",
				"cat":2
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 2</b>"
				]
			}
		}]
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"};
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


## 通过 SQL 高亮显示

`HIGHLIGHT()` 函数可用于高亮显示搜索结果。语法如下：

```sql
HIGHLIGHT([options], [field_list], [query] )
```

<!-- example highlight() no args -->
默认情况下，它在没有参数的情况下工作。

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
| 一扇门在他们面前打开，显露出一个小房间。                       |
+-----------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() field syntax -->

`HIGHLIGHT()` 从文档存储中检索所有可用的全文字段，并根据提供的查询进行高亮显示。查询中的字段语法是支持的。字段文本由 `field_separator` 分隔，可以在选项中修改。

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
| 书 <b>one</b> |
+-----------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() options -->
`HIGHLIGHT()` 的可选第一个参数是选项列表。

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
| 书 [match]one[/match]                                    |
+------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() field list -->

可选的第二个参数是一个包含单个字段或以逗号分隔的字段列表的字符串。如果该参数存在，则仅从文档存储中提取并高亮显示指定的字段。第二个参数为空字符串表示“提取所有可用字段”。

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
| 书 <b>one</b> | 他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在始终是一个不断感受到的威胁。 |
| 班德带领三人进入房间。一个机器人也跟了进来。班德示意其他机器人离开，自行走了进去。门在它身后关闭。 |
+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() string attr -->

另外，您可以使用第二个参数指定不带引号的字符串属性或字段名称。在这种情况下，提供的字符串将根据提供的查询进行高亮显示，但字段语法将被忽略。

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
| 书 <b>one</b>     |
| 书五               |
+---------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() query -->

可选的第三个参数是查询。这用于根据与搜索时使用的查询不同的查询来高亮显示搜索结果。

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
| 书 one                      |
| 书 <b>五</b>              |
+-------------------------------+
2 行集合 (0.00 秒)
```

<!-- end -->

<!-- example HIGHLIGHT TO_STRING -->

尽管 `HIGHLIGHT()` 旨在与存储的全文字段和字符串属性一起使用，但它也可以用于突出显示任意文本。请记住，如果查询包含任何字段搜索运算符（例如，`@title hello @body world`），则忽略它们的字段部分。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT({},TO_STRING('一些要突出显示的文本'), 'highlight') FROM books WHERE MATCH('@title one');
```

<!-- response SQL -->
```sql
+----------------------------------------------------------------+
| highlight({},TO_STRING('一些要突出显示的文本'), 'highlight') |
+----------------------------------------------------------------+
| 一些要突出显示的文本 <b>highlight</b>                                  |
+----------------------------------------------------------------+
1 行集合 (0.00 秒)
```

<!-- end -->

一些选项仅在生成单个字符串作为结果（而不是片段数组）时相关。这仅适用于 SQL `HIGHLIGHT()` 函数：

#### snippet_separator
插入片段之间的字符串。默认值为 ` ... `。
#### field_separator
插入字段之间的字符串。默认值为 `|`。


另一种突出显示文本的方式是使用 [CALL SNIPPETS](../Searching/Highlighting.md#CALL-SNIPPETS) 语句。这主要复制了 `HIGHLIGHT()` 的功能，但不能使用内置文档存储。不过，它可以从文件加载源文本。


## 通过 HTTP 突出显示

<!-- example highlight in JSON -->

要通过 HTTP 在 JSON 查询中突出显示全文搜索结果，字段内容必须存储在文档存储中（默认启用）。在此示例中，全文字段 `content` 和 `title` 从文档存储中获取，并针对 `query` 子句中指定的查询进行高亮显示。

突出显示的片段在 `hits` 数组的 `highlight` 属性中返回。

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
          "title": "书籍一",
          "content": "他们跟随着班德。机器人保持在礼貌的距离，但它们的存在始终是一个令人感到威胁的因素。班德将所有三人迎入房间。一个机器人也跟了进来。班德示意其他机器人离开，并自己进入。门在它身后关上。"
        },
        "highlight": {
          "content": [
            "他们跟随着班德。<b>机器人</b>保持在礼貌的距离，",
            "三人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德",
            "示意其他<b>机器人</b>离开并自己进入。"
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
    echo '文档: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "字段 ".$field." 的高亮:\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}

```

<!-- response PHP -->
```php
文档: 1
title : 书籍一
content : 他们跟随着班德。机器人保持在礼貌的距离，但它们的存在始终是一个令人感到威胁的因素。班德将所有三人迎入房间。一个机器人也跟了进来。班德示意其他机器人离开，并自己进入。门在它身后关上。
字段 content 的高亮:
- 他们跟随着班德。<b>机器人</b>保持在礼貌的距离，
- 三人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德
- 示意其他<b>机器人</b>离开并自己进入。

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
                    u'_source': {u'content': u'他们跟随着班德。机器人保持在礼貌的距离，但它们的存在始终是一个令人感到威胁的因素。班德将所有三人迎入房间。一个机器人也跟了进来。班德示意其他机器人离开，并自己进入。门在它身后关上。',
                                 u'title': u'书籍一'},
                    u'highlight': {u'content': [u'他们跟随着班德。<b>机器人</b>保持在礼貌的距离，',
                                                u'三人进入房间。<b>一个</b> <b>机器人</b>也跟了进来。班德',
                                                u'示意其他<b>机器人</b>离开并自己进入。']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍一","content":"他们跟随班德。机器人保持在适当的距离，但他们的存在始终构成威胁。班德把所有三人都带进了房间。一个机器人也跟了进来。班德示意其他机器人离开，自己进去了。门在它身后关上。 "},"highlight":{"content":["他们跟随班德。<b>机器人</b>保持在适当的距离，","三个进了房间。<b>一个</b><b>机器人</b>也跟了进来。班德","示意其他<b>机器人</b>离开，自己进去了。"]}}]}}
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
        hits: [{_id=1, _score=2788, _source={title=书籍一, content=他们跟随班德。机器人保持在适当的距离，但他们的存在始终构成威胁。班德把所有三人都带进了房间。一个机器人也跟了进来。班德示意其他机器人离开，自己进去了。门在它身后关上。 }, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。<b>机器人</b>保持在适当的距离， ,  三个进了房间。<b>一个</b>的<b>机器人</b>也跟了进来。班德,  示意其他<b>机器人</b>离开，自己进去了。]}}]
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
        hits: [{_id=1, _score=2788, _source={title=书籍一, content=他们跟随班德。机器人保持在适当的距离，但他们的存在始终构成威胁。班德把所有三人都带进了房间。一个机器人也跟了进来。班德示意其他机器人离开，自己进去了。门在它身后关上。 }, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。<b>机器人</b>保持在适当的距离， ,  三个进了房间。<b>一个</b>的<b>机器人</b>也跟了进来。班德,  示意其他<b>机器人</b>离开，自己进去了。]}}]
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
      *: '文本 1|文本 9'
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1|文本 9"};
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example highlight JSON all field  -->

要突出显示所有可能的字段，请将空对象作为 `highlight` 属性传递。

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
          "title": "书籍一",
          "content": "他们跟随班德。机器人保持在适当的距离，但他们的存在始终构成威胁。班德把所有三人都带进了房间。一个机器人也跟了进来。班德示意其他机器人离开，自己进去了。门在它身后关上。 "
        },
        "highlight": {
          "title": [
            "书籍 <b>一</b>"
          ],
          "content": [
            "他们跟随班德。<b>机器人</b>保持在适当的距离， ",
            " 三个进了房间。<b>一个</b>的<b>机器人</b>也跟了进来。班德",
            " 示意其他<b>机器人</b>离开，自己进去了。"
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
title : 书籍 一
content : 他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在是一种不断感受到的威胁。班德把所有三个人带入房间。一个机器人也跟随而来。班德把其他机器人示意离开，自己进入。门在后面关闭了。
Highlight for title:
- 书籍 <b>一</b>
Highlight for content:
- 他们跟随班德。<b>机器人</b>保持在一个礼貌的距离，
- 三个进入房间。<b>一个</b> <b>机器人</b>也跟随而来。班德
- 示意其他<b>机器人</b>离开，自己进入。  
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
                    u'_source': {u'content': u'他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在是一种不断感受到的威胁。班德把所有三个人带入房间。一个机器人也跟随而来。班德把其他机器人示意离开，自己进入。门在后面关闭了。',
                                 u'title': u'书籍 一'},
                    u'highlight': {u'content': [u'他们跟随班德。<b>机器人</b>保持在一个礼貌的距离，',
                                                u' 三个进入房间。<b>一个</b> <b>机器人</b>也跟随而来。班德',
                                                u' 示意其他<b>机器人</b>离开，自己进入。'],
                                   u'title': [u'书籍 <b>一</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍 一","content":"他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在是一种不断感受到的威胁。班德把所有三个人带入房间。一个机器人也跟随而来。班德把其他机器人示意离开，自己进入。门在后面关闭了。 "},"highlight":{"title":["书籍 <b>一</b>"],"content":["他们跟随班德。<b>机器人</b>保持在一个礼貌的距离，"," 三个进入房间。<b>一个</b> <b>机器人</b>也跟随而来。班德"," 示意其他<b>机器人</b>离开，自己进入。"]}}]}}

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
        hits: [{_id=1, _score=2788, _source={title=书籍 一, content=他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在是一种不断感受到的威胁。班德把所有三个人带入房间。一个机器人也跟随而来。班德把其他机器人示意离开，自己进入。门在后面关闭了。 }, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。<b>机器人</b>保持在一个礼貌的距离, ,  三个进入房间。<b>一个</b> <b>机器人</b>也跟随而来。班德,  示意其他<b>机器人</b>离开，自己进入。]} }]
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
        hits: [{_id=1, _score=2788, _source={title=书籍 一, content=他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在是一种不断感受到的威胁。班德把所有三个人带入房间。一个机器人也跟随而来。班德把其他机器人示意离开，自己进入。门在后面关闭了。 }, highlight={title=[书籍 <b>一</b>], content=[他们跟随班德。<b>机器人</b>保持在一个礼貌的距离, ,  三个进入房间。<b>一个</b> <b>机器人</b>也跟随而来。班德,  示意其他<b>机器人</b>离开，自己进入。]} }]
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
      *: '文本 1|文档 1'
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				],
				"name":
				[
					"<b>文档 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1|文档 1"};
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				],
				"name":
				[
					"<b>文档 1</b>"
				]
			}
		]}
	}
}
```


<!-- end -->

除了常见的高亮选项外，还通过 HTTP 为 JSON 查询提供了几个同义词：

#### fields
`fields` 对象包含具有选项的属性名称。它也可以是字段名称的数组（没有任何选项）。

请注意，默认情况下，高亮尝试高亮完全文本查询之后的结果。在一般情况下，当您不指定要高亮的字段时，高亮是基于您的完全文本查询进行的。但是，如果您指定了要高亮的字段，则只有在完全文本查询与所选字段匹配时才会高亮。

#### encoder
`encoder` 可以设置为 `default` 或 `html`。设置为 `html` 时，在高亮时保留 HTML 标记。这与 `html_strip_mode=retain` 选项的工作方式相似。

<!-- example highlight_query -->
#### highlight_query
`highlight_query` 选项允许您针对与搜索查询不同的查询进行高亮。语法与主 `query` 中相同。

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
    echo '文档: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "高亮 ".$field.":\n";
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
                    u'_source': {u'content': u'他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在是一个不断感受到的威胁。班德把三个人都引进了房间。一个机器人也跟随其后。班德示意其他机器人离开，自己也进去了。门在它身后关上。',
                                 u'title': u'书籍一'},
                    u'highlight': {u'content': [u'。机器人保持在一个 <b>礼貌的距离</b>，但他们的存在是一个']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1788,"_source":{"title":"书籍一","content":"他们跟随班德。机器人保持在一个礼貌的距离，但他们的存在是一个不断感受到的威胁。班德把三个人都引进了房间。一个机器人也跟随其后。班德示意其他机器人离开，自己也进去了。门在它身后关上。 "},"highlight":{"content":["。机器人保持在一个 <b>礼貌的距离</b>，但他们的存在是一个"]}}]}}
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
`pre_tags` 和 `post_tags` 设置高亮文本片段的开标签和闭标签。它们的功能类似于 `before_match` 和 `after_match` 选项。这些是可选的，默认值分别为 `<b>` 和 `</b>`。

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

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content","title"],"pre_tags":"before_","post_tags":"_after"}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"他们跟着班德。机器人的存在始终存在着一种微妙的威胁，保持着礼貌的距离。班德将三人引入房间。一台机器人也跟随而来。班德示意其他机器人离开，自己进入了房间。门在它身后关上了。 "},"highlight":{"content":["他们跟着班德。before_robots_after保持着礼貌的距离， ","三人进入房间。before_One_after的before_robots_after也跟随而来。班德","示意其他before_robots_after离开，自己进入了房间。"],"title":["Books before_one_after"]}}]}}
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

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: '文本 1'
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"before_文本 1_after"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"}
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"before_文本 1_after"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example no_match_size  -->
#### no_match_size
`no_match_size` 的功能类似于 `allow_empty` 选项。如果设置为 0，它的作用相当于 `allow_empty=1`，允许在无法生成片段时返回空字符串作为高亮结果。否则，将返回字段的开头。这是可选的，默认值为 1。

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
    echo '文档: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "高亮显示 ".$field.":\n";
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
content : 他们跟着班德。机器人的存在始终存在着一种微妙的威胁，保持着礼貌的距离。班德将三人引入房间。一台机器人也跟随而来。班德示意其他机器人离开，自己进入了房间。门在它身后关上了。
Highlight for content:
- 他们跟着班德。<b>robots</b>保持着礼貌的距离，
- 三人进入房间。<b>One</b>的<b>robots</b>也跟随而来。班德
- 示意其他<b>robots</b>离开，自己进入了房间。The
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
                    u'_source': {u'content': u'他们追随班德。机器人保持在礼貌的距离，但它们的存在始终是一种威胁。班德将三人引入房间。一台机器人也跟了上来。班德示意其他机器人离开，然后自己进入。门在它的身后关闭。',
                                 u'title': u'书籍一'},
                    u'highlight': {u'content': [u'他们追随班德。<b>机器人</b>保持在礼貌的距离，',
                                                u'三人进入房间。<b>一台</b><b>机器人</b>也跟了上来。班德',
                                                u'示意其他<b>机器人</b>离开，然后自己进入。门'],
                                   u'title': [u'书籍<b>一</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"他们追随班德。机器人保持在礼貌的距离，但它们的存在始终是一种威胁。班德将三人引入房间。一台机器人也跟了上来。班德示意其他机器人离开，然后自己进入。门在它的身后关闭。 "},"highlight":{"content":["他们追随班德。<b>机器人</b>保持在礼貌的距离， ","三人进入房间。<b>一台</b><b>机器人</b>也跟了上来。班德","示意其他<b>机器人</b>离开，然后自己进入。门"],"title":["书籍 <b>一</b>"]}}]}}

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

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: '文本 1'
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"};
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example order  -->
#### order
`order` sets the sorting order of extracted snippets. If set to `"score"`, it sorts the extracted snippets in order of relevance. This is optional and works similarly to the `weight_order` option.

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
    echo '文档: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "高亮显示 ".$field.":\n";
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
title : 书籍一
content : 他们追随班德。机器人保持在礼貌的距离，但它们的存在始终是一种威胁。班德将三人引入房间。一台机器人也跟了上来。班德示意其他机器人离开，然后自己进入。门在它的身后关闭。
Highlight for content:
-  三人进入房间。<b>一台</b><b>机器人</b>也跟了上来。班德
- 指示其他<b>机器人</b>离开并进入自己。该
- 他们跟随班德。<b>机器人</b>保持在礼貌的距离，
为标题高亮：
- 书籍<b>一</b>
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
                    u'_source': {u'content': u'他们跟随班德。机器人保持在礼貌的距离，但他们的存在是一种时刻感受到的威胁。班德把所有三个人都带进了房间。其中一个机器人也跟了进来。班德指示其他机器人离开并自己进入。门在它身后关上。',
                                 u'title': u'书籍 一'},
                    u'highlight': {u'content': [u' 三个进入房间。<b>一个</b>的<b>机器人</b>也跟了进来。班德',
                                                u' 指示其他<b>机器人</b>离开并进入自己。该',
                                                u'他们跟随班德。<b>机器人</b>保持在礼貌的距离，'],
                                   u'title': [u'书籍 <b>一</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍 一","content":"他们跟随班德。机器人保持在礼貌的距离，但他们的存在是一种时刻感受到的威胁。班德把所有三个人都带进了房间。其中一个机器人也跟了进来。班德指示其他机器人离开并自己进入。门在它身后关上。 "},"highlight":{"content":[" 三个进入房间。<b>一个</b>的<b>机器人</b>也跟了进来。班德"," 指示其他<b>机器人</b>离开并进入自己。该","他们跟随班德。<b>机器人</b>保持在礼貌的距离， "],"title":["书籍 <b>一</b>"]}}]}}


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

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: '文本 1'
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "文本 1"};
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
				"content":"文本 1",
				"name":"文档 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本 1</b>"
				]
			}
		]}
	}
}
```


<!-- end -->

<!-- example fragment_size -->
#### fragment_size
`fragment_size` 设置最大片段大小，以符号为单位。它可以是全局的或每个字段的。每个字段的选项会覆盖全局选项。这是可选的，默认值为256。它的工作方式类似于`limit`选项。

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
    echo '文档: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "高亮显示 ".$field.":\n";
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
`number_of_fragments` 限制结果中代码片段的最大数量。与 `fragment_size` 类似，它可以全局设置或针对每个字段单独设置。这是可选的，默认值为 0（无限制）。其工作方式类似于 `limit_snippets` 选项。

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
像 `limit`、`limit_words` 和 `limit_snippets` 这样的选项可以设置为全局或每字段的选项。除非每字段的选项覆盖它们，否则全局选项会作为每字段的限制使用。在示例中，`title` 字段采用了默认的限制设置，而 `content` 字段则使用了不同的限制。
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
title : 书籍一
content : 他们跟随着班德尔。机器人保持着礼貌的距离，但他们的存在是一种持续感受到的威胁。班德尔带领三人进入房间。一个机器人也跟了进来。班德尔示意其他机器人离开，自己进入。门在他身后关上。
Highlight for content:
-  进入房间。<b>一个</b>机器人<b>跟了</b>进来
Highlight for title:
- 书籍 <b>一</b>
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
                    u'_source': {u'content': u'他们跟随着班德尔。机器人保持着礼貌的距离，但他们的存在是一种持续感受到的威胁。班德尔带领三人进入房间。一个机器人也跟了进来。班德尔示意其他机器人离开，自己进入。门在他身后关上。',
                                 u'title': u'书籍一'},
                    u'highlight': {u'content': [u' 进入房间。<b>一个</b>机器人<b>跟了</b>进来'],
                                   u'title': [u'书籍 <b>一</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"书籍一","content":"他们跟随着班德尔。机器人保持着礼貌的距离，但他们的存在是一种持续感受到的威胁。班德尔带领三人进入房间。一个机器人也跟了进来。班德尔示意其他机器人离开，自己进入。门在他身后关上。"},"highlight":{"title":["书籍 <b>一</b>"],"content":[" 进入房间。<b>一个</b>机器人<b>跟了</b>进来"]}}]}}
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
				"content":"文本1",
				"name":"文档1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>文本</b>"
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
				"content":"文本1",
				"name":"文档1",
				"cat":1
			},
			"highlight":
			{
		"content":
				[
					"<b>文本</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example highlight json global limits -->

#### limits_per_field
全局限制也可以通过指定 `limits_per_field=0` 来强制执行。设置此选项意味着所有组合的高亮结果必须在指定的限制范围内。缺点是，如果高亮引擎决定某些内容更相关，可能会在一个字段中高亮多个片段，而在另一个字段中没有高亮。

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

# CALL SNIPPETS

<!-- example CALL SNIPPETS -->

`CALL SNIPPETS` 语句使用指定的表设置从提供的数据和查询构建一个代码段。它无法访问内置文档存储，这就是为什么建议使用 [HIGHLIGHT() 函数](../Searching/Highlighting.md)代替。

语法是：

```sql
CALL SNIPPETS(data, table, query[, opt_value AS opt_name[, ...]])
```

#### data
`data` 作为提取片段的来源。它可以是一个单独的字符串，也可以是用大括号括起来的字符串列表。
#### table
`table` 指的是提供文本处理设置以生成片段的表的名称。
#### query
`query` 是用于构建片段的全文查询。
#### opt_value 和 opt_name
`opt_value` 和 `opt_name` 代表 [片段生成选项](../Searching/Highlighting.md)。

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

大多数选项与 [HIGHLIGHT() 函数](../Searching/Highlighting.md) 中的相同。 然而，有几个选项只能与 `CALL SNIPPETS` 一起使用。

<!-- example CALL SNIPPETS load files -->
以下选项可用于突出显示存储在单独文件中的文本：

#### load_files
此选项启用时，将第一个参数视为文件名，而不是要提取片段的数据。 服务器端指定的文件将被加载为数据。 每个请求最多将使用 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) 个工作线程来并行化工作，当此标志启用时。 默认值为 0（无限制）。 若要在远程代理之间分配片段生成，请在仅包含一个（！）本地代理和多个远程代理的分布式表中调用片段生成。 [snippets_file_prefix](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#snippets_file_prefix) 选项用于生成最终的文件名。 例如，当 searchd 配置为 `snippets_file_prefix = /var/data_` 且 `text.txt` 被提供为文件名时，片段将从 `/var/data_text.txt` 的内容中生成。

#### load_files_scattered
此选项仅适用于具有远程代理的分布式片段生成。 片段生成的源文件可以在不同的代理之间分配，主服务器将合并所有无错误的结果。 例如，如果分布式表的一个代理有 `file1.txt`，另一个代理有 `file2.txt`，而您使用 `CALL SNIPPETS` 同时使用这两个文件，searchd 将合并代理结果，因此您将从 `file1.txt` 和 `file2.txt` 中获得结果。 默认值为 0。

如果 `load_files` 选项也启用，则如果任何文件在任何地方不可用，请求将返回错误。 否则（如果未启用 `load_files`），它将返回所有缺失文件的空字符串。 Searchd 不会将此标志传递给代理，因此如果文件不存在，代理不会生成关键错误。 如果您想确保加载所有源文件，请将 `load_files_scattered` 和 `load_files` 都设置为 1。 如果某些代理上的某些源文件缺失不是关键，则仅将 `load_files_scattered` 设置为 1。

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
