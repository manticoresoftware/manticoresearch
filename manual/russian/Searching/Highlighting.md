# Подсветка

<!-- пример подсветки -->

Подсветка позволяет вам получать фрагменты текста с подсветкой (называемые сниппетами) из документов, содержащих соответствующие ключевые слова.

Функция SQL `HIGHLIGHT()`, свойство `"highlight"` в JSON-запросах через HTTP и функция `highlight()` в PHP-клиенте все используют встроенное хранилище документов для извлечения оригинального содержимого поля (включено по умолчанию).

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
| Не <b>пытайтесь</b> соревноваться в детскости, сказал Блис. |
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
          "title":"Книга четвертая",
          "content":"Не пытайтесь соревноваться в детскости, сказал Блис."
        },
        "highlight":
        {
          "title": ["Книга четвертая"],
          "content": ["Не <b>пытайтесь</b> соревноваться в детскости, сказал Блис."]
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
    echo 'Документ: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Подсветка для ".$field.":
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
Документ: 14
title: Книга четвертая
content: Не пытайтесь соревноваться в детскости, сказал Блис.
Подсветка для title:
- Книга четвертая
Подсветка для content:
- Не <b>пытайтесь</b> соревноваться в детскости, сказал Блис.

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
                    u'_source': {u'content': u'Не пытайтесь соревноваться в детскости, сказал Блис.',
                                 u'title': u'Книга четвертая'},
                    u'highlight': {u'content': [u'Не <b>пытайтесь</b> соревноваться в детскости, сказал Блис.'],
                                   u'title': [u'Книга четвертая']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 4,"_score":1695,"_source":{"title":"Книга четвертая","content":"Не пытайтесь соревноваться в детскости, сказал Блис."},"highlight":{"title":["Книга четвертая"],"content":["Не <b>пытайтесь</b> соревноваться в детскости, сказал Блис."]}}]}}
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
        hits: [{_id=3, _score=1597, _source={title=Книга третья, content=Тревиз шептал, "Это извлекает детское удовольствие из выставления. Мне бы хотелось его сбить."}, highlight={title=[Книга третья], content=[, "Это <b>извлекает</b> детское удовольствие ,  сбить его <b>сбить</b>."]}}, {_id=4, _score=1563, _source={title=Книга четвертая, content=Не пытайтесь соревноваться в детскости, сказал Блис.}, highlight={title=[Книга четвертая], content=[Не <b>пытайтесь</b> соревноваться в детскости, <b>сказал</b> Блис.]}}, {_id=5, _score=1514, _source={title=Книги две, content=Дверь открылась перед ними, открывая маленькую комнату. Бандер сказал: "Идите, полулюди, я хочу показать вам, как мы живем."}, highlight={title=[Книги две], content=[ маленькая комната. Бандер <b>сказал</b>, "Идите, полулюди,"]}]
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
        hits: [{_id=3, _score=1597, _source={title=Книга три, content=Тревиз прошептал: "Она получает детское удовольствие от зрелища. Я бы с удовольствием сбил её."}, highlight={title=[Книга три], content=[, "Она <b>получает</b> детское удовольствие ,  сбить её <b>сбить</b>."]}}, {_id=4, _score=1563, _source={title=Книга четыре, content=Не пытайтесь соревноваться в детскости, сказал Блисс.}, highlight={title=[Книга четыре], content=[Не <b>пытайтесь</b> соревноваться в детскости, <b>сказал</b> Блисс.]}}, {_id=5, _score=1514, _source={title=Книги два, content=Дверь открылась перед ними, открывая маленькую комнату. Бандер сказал: "Идём, полугуманоиды, я хочу показать вам, как мы живём."}, highlight={title=[Книги два], content=[ маленькая комната. Бандер <b>сказал</b>: "Идём, полугуманоиды, я]}}]
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
      *: 'Текст 1'
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
				"content":"Текст 1"
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1"};
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
				"content":"Текст 1"
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

При использовании SQL для выделения результатов поиска вы получите фрагменты из различных полей, объединенные в одну строку из-за ограничений протокола MySQL. Вы можете настроить разделители конкатенации с помощью параметров `field_separator` и `snippet_separator`, как описано ниже.

При выполнении JSON-запросов через HTTP или с использованием PHP-клиента таких ограничений нет, и набор результатов включает массив полей, содержащих массивы фрагментов (без разделителей).

Имейте в виду, что такие параметры генерации фрагментов, как `limit`, `limit_words` и `limit_snippets`, применяются к каждому полю отдельно по умолчанию. Вы можете изменить это поведение с помощью параметра `limits_per_field`, но это может привести к нежелательным результатам. Например, одно поле может иметь совпадающие ключевые слова, но в набор результатов не будут включены фрагменты из этого поля, потому что они не оценивались так высоко, как фрагменты из других полей в движке выделения.

Алгоритм выделения в настоящее время приоритизирует лучшие фрагменты (с более близкими соответствиями фраз) и затем фрагменты с ключевыми словами, которые еще не были включены в результат. В общем, он направлен на выделение лучшего совпадения для запроса и выделение всех ключевых слов запроса, как это позволяет делать лимиты. Если совпадений не найдено в текущем поле, начало документа будет обрезано в соответствии с лимитами и возвращено по умолчанию. Чтобы вернуть пустую строку вместо этого, установите параметр `allow_empty` в 1.

Выделение выполняется в так называемой стадии `post limit`, что означает, что генерация фрагментов откладывается не только до тех пор, пока весь окончательный набор результатов не будет подготовлен, но и после применения условия LIMIT. Например, с условием LIMIT 20,10 функция `HIGHLIGHT()` будет вызываться максимум 10 раз.

## Опции выделения

<!-- example highlighting options -->

Существует несколько дополнительных опций выделения, которые можно использовать для тонкой настройки генерации фрагментов, которые общие для SQL, HTTP и PHP клиентов.

#### before_match
Строка, которую нужно вставить перед совпадением ключевого слова. Макрос `%SNIPPET_ID%` может быть использован в этой строке. Первое вхождение макроса заменяется на увеличивающийся номер фрагмента в текущем фрагменте. Нумерация начинается с 1 по умолчанию, но может быть переопределена с помощью параметра `start_snippet_id`. %SNIPPET_ID% перезапускается в начале каждого нового документа. Значение по умолчанию - `<b>`.

#### after_match
Строка, которую нужно вставить после совпадения ключевого слова. Значение по умолчанию - `</b>`.

#### limit
Максимальный размер фрагмента, в символах (кодовых точках). Значение по умолчанию - 256. Это применяется к каждому полю по умолчанию, см. `limits_per_field`.

#### limit_words
Ограничивает максимальное количество слов, которые могут быть включены в результат. Обратите внимание, что это ограничение применяется ко всем словам, а не только к совпадающим ключевым словам для выделения. Например, если выделяется `Мэри`, и выбирается фрагмент `Мэри имела маленького ягненка`, он вносит 5 слов в это ограничение, а не только 1. Значение по умолчанию - 0 (без ограничений). Это применяется к каждому полю по умолчанию, см. `limits_per_field`.

#### limit_snippets
Ограничивает максимальное количество фрагментов, которые могут быть включены в результат. Значение по умолчанию - 0 (без ограничений). Это применяется к каждому полю по умолчанию, см. `limits_per_field`.

#### limits_per_field
Определяет, будут ли `limit`, `limit_words` и `limit_snippets` работать как индивидуальные лимиты в каждом поле документа, который выделяется, или как глобальные лимиты для всего документа. Установка этого параметра в 0 означает, что все комбинированные результаты выделения для одного документа должны находиться в пределах указанных лимитов. Недостаток в том, что у вас может быть несколько фрагментов, выделенных в одном поле и ни одного в другом, если движок выделения решит, что они более актуальны. Значение по умолчанию - 1 (используйте периметровые лимиты).
#### around
Количество слов, которые нужно выбрать вокруг каждого совпадающего блока ключевых слов. Значение по умолчанию - 5.

#### use_boundaries
Определяет, следует ли дополнительно разбивать фрагменты по границам фраз, как задано в настройках таблицы с директивой [phrase_boundary](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary). Значение по умолчанию - 0 (не использовать границы).

#### weight_order
Указывает, нужно ли сортировать извлеченные фрагменты в порядке релевантности (убывающий вес) или в порядке появления в документе (возрастающая позиция). Значение по умолчанию - 0 (не использовать порядок весов).

#### force_all_words
Игнорирует ограничение по длине, пока результат не включает все ключевые слова. Значение по умолчанию - 0 (не принуждать к наличию всех ключевых слов).

#### start_snippet_id
Устанавливает начальное значение макроса `%SNIPPET_ID%` (который обнаруживается и расширяется в строках `before_match`, `after_match`). Значение по умолчанию - 1.

#### html_strip_mode
Определяет режим настройки удаления HTML. По умолчанию - `index`, что означает, что будут использоваться настройки таблицы. Другие значения включают `none` и `strip`, которые принудительно пропускают или применяют удаление независимо от настроек таблицы; и `retain`, который сохраняет HTML-разметку и защищает ее от подсветки. Режим `retain` может использоваться только при подсветке полных документов и, следовательно, требует, чтобы не было установлено ограничений на размер фрагмента. Допустимые строковые значения - `none`, `strip`, `index` и `retain`.

#### allow_empty
Разрешает возвращать пустую строку в качестве результата подсветки, когда в текущем поле не удалось создать фрагменты (нет совпадений по ключевым словам или ни один из фрагментов не соответствует лимиту). По умолчанию вместо пустой строки будет возвращено начало оригинального текста. Значение по умолчанию - 0 (не разрешать пустой результат).

#### snippet_boundary
Обеспечивает, чтобы фрагменты не пересекали границы предложений, абзацев или зон (при использовании с таблицей, у которой включены соответствующие настройки индексации). Допустимые значения - `sentence`, `paragraph`, и `zone`.

#### emit_zones
Генерирует HTML-тег с именем окружающей зоны перед каждым фрагментом. Значение по умолчанию - 0 (не генерировать имена зон).

#### force_snippets
Определяет, следует ли принудительно генерировать фрагменты, даже если лимиты позволяют выделять весь текст. Значение по умолчанию - 0 (не принуждать к генерации фрагментов).

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
Не <b>пытайтесь</b> соревноваться в детскости, <b>сказал</b> Блисс.

Document: 2
title: Книга два
content: Дверь открылась перед ними, открывая небольшую комнату. Бандер сказал: "Идите, полулюди, я хочу показать вам, как мы живем.
Highlight for title:
- Книга два
Highlight for content:
 небольшую комнату. Бандер <b>сказал</b>: "Идите, полулюди, я
```

<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"пытайтесь"}},"highlight":{"limit":50}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'4',
                    u'_score': 1695,
                    u'_source': {u'content': u'Не пытайтесь соревноваться в детскости, сказал Блисс.',
                                 u'title': u'Книга четыре'},
                    u'highlight': {u'content': [u'Не <b>пытайтесь</b> соревноваться в детскости, сказал Блисс.'],
                                   u'title': [u'Книга четыре']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"query_string":"пытайтесь|получает|свалить|сказал"},"highlight":{"limit":50}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":3,"hits":[{"_id": 3,"_score":1597,"_source":{"title":"Книга три","content":"Тревиз шепнул: "Это получает Infantile удовольствие от показа. Я бы хотел сбить это.""},"highlight":{"title":["Книга три"],"content":[", "Это <b>получает</b> Infantile удовольствие "," чтобы сбить его <b>свалить</b>.""]}},{"_id": 4,"_score":1563,"_source":{"title":"Книга четыре","content":"Не пытайтесь соревноваться в детскости, сказал Блисс."},"highlight":{"title":["Книга четыре"],"content":["Не <b>пытайтесь</b> соревноваться в детскости, <b>сказал</b> Блисс."]}},{"_id": 5,"_score":1514,"_source":{"title":"Книга два","content":"Дверь открылась перед ними, открывая небольшую комнату. Бандер сказал: "Идите, полулюди, я хочу показать вам, как мы живем.""},"highlight":{"title":["Книга два"],"content":[" небольшую комнату. Бандер <b>сказал</b>: "Идите, полулюди, я"]}}]}}
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","пытайтесь|получает|свалить|сказал");
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
        hits: [{_id=3, _score=1597, _source={title=Книга три, content=Тревиз шепнул, "Это получает Infantile удовольствие от показа. Я бы хотел сбить это."}, highlight={title=[Книга три], content=[, "Это <b>получает</b> Infantile удовольствие ,  чтобы сбить его <b>свалить</b>."]}}, {_id=4, _score=1563, _source={title=Книга четыре, content=Не пытайтесь соревноваться в детскости, сказал Блисс.}, highlight={title=[Книга четыре], content=[Не <b>пытайтесь</b> соревноваться в детскости, <b>сказал</b> Блисс.]}}, {_id=5, _score=1514, _source={title=Книга два, content=Дверь открылась перед ними, открывая небольшую комнату. Бандер сказал: "Идите, полулюди, я хочу показать вам, как мы живем."}, highlight={title=[Книга два], content=[ небольшую комнату. Бандер <b>сказал</b>: "Идите, полулюди, я]}}]
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
searchRequest.FulltextFilter = new MatchFilter("*", "пытайтесь|получает|свалить|сказал");
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
        hits: [{_id=3, _score=1597, _source={title=Книга три, content=Тревиз шепнул, "Это получает Infantile удовольствие от показа. Я бы хотел сбить это."}, highlight={title=[Книга три], content=[, "Это <b>получает</b> Infantile удовольствие ,  чтобы сбить его <b>свалить</b>."]}}, {_id=4, _score=1563, _source={title=Книга четыре, content=Не пытайтесь соревноваться в детскости, сказал Блисс.}, highlight={title=[Книга четыре], content=[Не <b>пытайтесь</b> соревноваться в детскости, <b>сказал</b> Блисс.]}}, {_id=5, _score=1514, _source={title=Книга два, content=Дверь открылась перед ними, открывая небольшую комнату. Бандер сказал: "Идите, полулюди, я хочу показать вам, как мы живем."}, highlight={title=[Книга два], content=[ небольшую комнату. Бандер <b>сказал</b>: "Идите, полулюди, я]}}]
        aggregations: null
    }
    profile: null
}
```
<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: { match: { *: 'Текст } },
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		},
		{
			"_id": 2,
			"_score":1480,
			"_source":
			{
				"content":"Текст 2",
				"name":"Док 2",
				"cat":2
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 2</b>"
				]
			}
		}]
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1"};
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
				"content":"Текст 1",
				"name":"Документ 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		},
		{
			"_id": 2,
			"_score":1480,
			"_source":
			{
				"content":"Текст 2",
				"name":"Документ 2",
				"cat":2
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 2</b>"
				]
			}
		}]
	}
}
```


<!-- end -->


## Выделение с помощью SQL

Функция `HIGHLIGHT()` может использоваться для выделения результатов поиска. Вот синтаксис:

```sql
HIGHLIGHT([options], [field_list], [query] )
```

<!-- example highlight() no args -->
По умолчанию она работает без аргументов.

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
| Дверь открылась <b>до</b> них, открывая маленькую комнату. |
+-----------------------------------------------------------+
1 строка в наборе (0.00 сек)
```

<!-- end -->

<!-- example highlight() field syntax -->

`HIGHLIGHT()` извлекает все доступные полнотекстовые поля из хранилища документов и выделяет их по сравнению с предоставленным запросом. Синтаксис полей в запросах поддерживается. Текст поля разделяется `field_separator`, который можно изменить в параметрах.

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
| Книга <b>один</b> |
+-----------------+
1 строка в наборе (0.00 сек)
```

<!-- end -->

<!-- example highlight() options -->
Необязательный первый аргумент в `HIGHLIGHT()` — это список параметров.

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
| Книга [match]один[/match]                                   |
+------------------------------------------------------------+
1 строка в наборе (0.00 сек)
```

<!-- end -->

<!-- example highlight() field list -->

Второй аргумент — это строка, содержащая одно поле или список полей, разделенных запятыми. Если этот аргумент указан, будут извлечены только указанные поля из хранилища документов и выделены. Пустая строка в качестве второго аргумента означает «извлечь все доступные поля».

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
| Книга <b>один</b> | Они шли за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, но их присутствие было постоянно ощутимой угрозой.                                               |
| Бандер ввел всех троих в комнату. <b>Один</b> из <b>роботов</b> также последовал за ним. Бандер жестами указал другим <b>роботам</b> удалиться и сам вошел. Дверь закрылась за ним. |
+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
2 строки в наборе (0.00 сек)
```

<!-- end -->

<!-- example highlight() string attr -->

В качестве второго аргумента вы можете указать строковый атрибут или имя поля без кавычек. В этом случае предоставленная строка будет выделена по сравнению с предоставленным запросом, но синтаксис поля будет проигнорирован.

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
| Книга <b>один</b>     |
| Книга пять           |
+---------------------+
2 строки в наборе (0.00 сек)
```

<!-- end -->

<!-- example highlight() query -->

Необязательный третий аргумент — это запрос. Он используется для выделения результатов поиска по запросу, отличному от того, который использовался для поиска.

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
| Книга один                    |
| Книга <b>пять</b>              |
+-------------------------------+
2 строки в множестве (0.00 сек)
```

<!-- end -->

<!-- example HIGHLIGHT TO_STRING -->

Хотя `HIGHLIGHT()` предназначена для работы с сохранёнными полными текстовыми полями и строковыми атрибутами, её также можно использовать для выделения произвольного текста. Имейте в виду, что если запрос содержит операторы поиска по полям (например, `@title hello @body world`), часть поля в них игнорируется в этом случае.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT({},TO_STRING('некоторый текст для выделения'), 'highlight') FROM books WHERE MATCH('@title один');
```

<!-- response SQL -->
```sql
+----------------------------------------------------------------+
| highlight({},TO_STRING('некоторый текст для выделения'), 'highlight') |
+----------------------------------------------------------------+
| некоторый текст для <b>выделения</b>                                  |
+----------------------------------------------------------------+
1 строка в множестве (0.00 сек)
```

<!-- end -->

Несколько опций актуальны только при генерации одной строки в качестве результата (не массива фрагментов). Это относится исключительно к функции SQL `HIGHLIGHT()`:

#### snippet_separator
Строка для вставки между фрагментами. По умолчанию ` ... `.
#### field_separator
Строка для вставки между полями. По умолчанию `|`.


Другой способ выделять текст - использовать оператор [CALL SNIPPETS](../Searching/Highlighting.md#CALL-SNIPPETS). Это в основном дублирует функционал `HIGHLIGHT()`, но не может использовать встроенное хранение документов. Однако он может загружать исходный текст из файлов.


## Выделение через HTTP

<!-- example highlight in JSON -->

Чтобы выделять результаты полнотекстового поиска в JSON-запросах через HTTP, содержимое полей должно храниться в хранилище документов (включено по умолчанию). В примере полнотекстовые поля `content` и `title` извлекаются из хранилища документов и выделяются по запросу, указанному в разделе `query`.

Выделенные фрагменты возвращаются в свойстве `highlight` массива `hits`.

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "*": "один|роботы" } },
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
          "title": "Книги один",
          "content": "Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянно ощутимой угрозой. Бандер ввёл всех троих в комнату. Один из роботов также следовал. Бандер жестом указал другим роботам отойти и вошёл сам. Дверь закрылась за ним. "
        },
        "highlight": {
          "content": [
            "Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ",
            " троих в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер",
            " жестом указал другим <b>роботам</b> отойти и вошёл сам. "
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
$results = $index->search('один|роботы')->highlight(['content'])->get();
foreach($results as $doc)
{
    echo 'Документ: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Выделение для ".$field.":
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
Документ: 1
title : Книги один
content : Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянно ощутимой угрозой. Бандер ввёл всех троих в комнату. Один из роботов также следовал. Бандер жестом указал другим роботам отойти и вошёл сам. Дверь закрылась за ним.
Выделение для content:
- Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии,
-  троих в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер
-  жестом указал другим <b>роботам</b> отойти и вошёл сам. 
```
<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"один|роботы"}},"highlight":{"fields":["content"]}}))
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянно ощутимой угрозой. Бандер ввёл всех троих в комнату. Один из роботов также следовал. Бандер жестом указал другим роботам отойти и вошёл сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u'Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ',
                                                u' троих в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер',
                                                u' жестом указал другим <b>роботам</b> отойти и вошёл сам. ']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"один|роботы"}},"highlight":{"fields":["content"]}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер проводил их троих в комнату. Один из роботов также следовал за ними. Бандер отогнал остальных роботов и сам вошел. Дверь закрылась за ним. "},"highlight":{"content":["Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, "," троих в комнату. <b>Один</b> из <b>роботов</b> также следовал за ними. Бандер"," отогнал остальных <b>роботов</b> и сам вошел. Дверь"]}}]}}
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
        hits: [{_id=1, _score=2788, _source={title=Книги один, content=Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер проводил их троих в комнату. Один из роботов также следовал за ними. Бандер отогнал остальных роботов и сам вошел. Дверь закрылась за ним. }, highlight={title=[Книги <b>один</b>], content=[Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ,  троих в комнату. <b>Один</b> из <b>роботов</b> также следовал за ними. Бандер,  отогнал остальных <b>роботов</b> и сам вошел. Дверь]}]
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
        hits: [{_id=1, _score=2788, _source={title=Книги один, content=Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер проводил их троих в комнату. Один из роботов также следовал за ними. Бандер отогнал остальных роботов и сам вошел. Дверь закрылась за ним. }, highlight={title=[Книги <b>один</b>], content=[Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ,  троих в комнату. <b>Один</b> из <b>роботов</b> также следовал за ними. Бандер,  отогнал остальных <b>роботов</b> и сам вошел. Дверь]}]
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
      *: 'Текст 1|Текст 9'
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1|Текст 9"};
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example highlight JSON all field  -->

Чтобы выделить все возможные поля, передайте пустой объект в свойство `highlight`.

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
          "title": "Книги один",
          "content": "Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер проводил их троих в комнату. Один из роботов также следовал за ними. Бандер отогнал остальных роботов и сам вошел. Дверь закрылась за ним. "
        },
        "highlight": {
          "title": [
            "Книги <b>один</b>"
          ],
          "content": [
            "Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ",
            " троих в комнату. <b>Один</b> из <b>роботов</b> также следовал за ними. Бандер",
            " отогнал остальных <b>роботов</b> и сам вошел. Дверь"
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

title : Книги один

content : Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал за ними. Бандер указал другим роботам уйти и вошел сам. Дверь закрылась за ним.

Highlight for title:

- Книги <b>один</b>

Highlight for content:

- Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии,

-  трое в комнату. <b>Один</b> из <b>роботов</b> тоже последовал за ними. Бандер

-  указал другим <b>роботам</b> уйти и вошел сам. The

```

<!-- request Python -->

``` python

res = searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал за ними. Бандер указал другим.robot(сам. Дверь closed за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u'Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ',
                                                u' трое в комнату. <b>Один</b> из <b>роботов</b> тоже последовал за ними. Бандер',
                                                u' указал другим <b>роботам</b> уйти и вошел сам. The'],
                                   u'title': [u'Книги <b>один</b>']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{} );
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал за ними. Бандер указал другим.robot(сам. Дверь closed за ним. "},"highlight":{"title":["Книги <b>один</b>"],"content":["Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, "," трое в комнату. <b>Один</b> из <b>роботов</b> тоже последовал за ними. Бандер"," указал другим <b>роботам</b> уйти и вошел сам. The"]}}]}}
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

        hits: [{_id=1, _score=2788, _source={title=Книги один, content=Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал за ними. Бандер указал другим.robot(сам. Дверь closed за ним. }, highlight={title=[Книги <b>один</b>], content=[Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ,  трое в комнату. <b>Один</b> из <b>роботов</b> тоже последовал за ними. Бандер,  указал другим <b>роботам</b> уйти и вошел сам. The]}}]

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

        hits: [{_id=1, _score=2788, _source={title=Книги один, content=Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал за ними. Бандер указал другим.robot(сам. Дверь closed за ним. }, highlight={title=[Книги <b>один</b>], content=[Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ,  трое в комнату. <b>Один</b> из <b>роботов</b> тоже последовал за ними. Бандер,  указал другим <b>роботам</b> уйти и вошел сам. The]}}]

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
      *: 'Текст 1|Док 1'
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				],
				"name":
				[
					"<b>Док 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1|Док 1"};
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				],
				"name":
				[
					"<b>Док 1</b>"
				]
			}
		]}
	}
}
```


<!-- end -->

В дополнение к обычным параметрам подсветки, несколько синонимов доступны для JSON-запросов через HTTP:

#### fields
Объект `fields` содержит имена атрибутов с параметрами. Это также может быть массив имен полей (без каких-либо параметров).

Обратите внимание, что по умолчанию подсветка пытается выделить результаты, следуя запросу по полному тексту. В общем случае, когда вы не указываете поля для подсветки, подсветка базируется на вашем запросе по полному тексту. Однако, если вы указываете поля для подсветки, она выделяется только в том случае, если запрос по полному тексту соответствует выбранным полям.

#### encoder
Параметр `encoder` может быть установлен в значение `default` или `html`. При установке в значение `html`, он сохраняет HTML-разметку при подсветке. Это работает аналогично опции `html_strip_mode=retain`.

<!-- example highlight_query -->
#### highlight_query
Параметр `highlight_query` позволяет вам подсвечивать по запросу, отличному от вашего поискового запроса. Синтаксис такой же, как и в основном `query`.

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
    "highlight_query": { "match": { "*":"вежливое расстояние" } }
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

$results = $index->search($bool)->highlight(['content'],['highlight_query'=>['match'=>['*'=>'вежливое расстояние']]])->get();
foreach($results as $doc)
{
    echo 'Документ: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Подсветка для ".$field.":
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
res = searchApi.search({"table":"books","query":{"match":{"content":"one|robots"}},"highlight":{"fields":["content"],"highlight_query":{"match":{"*":"вежливое расстояние"}}}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1788,
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы держались на вежливом расстоянии, но их присутствие стало постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов последовал за ними. Бандер жестом отослал остальных роботов и вошел сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u'. Роботы держались на <b>вежливом расстоянии</b>, но их присутствие стало']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"content":"one|robots"}},"highlight":{"fields":["content"],"highlight_query":{"match":{"*":"вежливое расстояние"}}}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1788,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы держались на вежливом расстоянии, но их присутствие стало постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов последовал за ними. Бандер жестом отослал остальных роботов и вошел сам. Дверь закрылась за ним. "},"highlight":{"content":[". Роботы держались на <b>вежливом расстоянии</b>, но их присутствие стало"]}}]}}
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
            put("*","вежливое расстояние");
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
match.Add("*", "вежливое расстояние");
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
      *: 'Текст 1'
    }
  },
  highlight: {
    fields: ['content'],
    highlight_query: {
      match: {*: 'Текст'}
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст</b> 1"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlightField := manticoreclient.NetHighlightField("content")
highlightFields := []interface{} { highlightField } 
highlight.SetFields(highlightFields)
queryMatchClause := map[string]interface{} {"*": "Текст"};
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст</b> 1"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example pre_tags  -->

#### pre_tags and post_tags
`pre_tags` and `post_tags` устанавливают открывающие и закрывающие теги для выделенных фрагментов текста. Они функционируют аналогично параметрам `before_match` и `after_match`. Эти параметры являются необязательными, с значениями по умолчанию `<b>` и `</b>`.

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
    echo 'Документ: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Выделение для ".$field.":
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
Документ: 1
title : Книги один
content : Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер жестом отогнал других роботов и вошел сам. Дверь закрылась за ним.
Выделение для content:
- Они следовали за Бандером. Роботы before_остались_after на вежливом расстоянии,
-  троих в комнату. before_Один_after из перед_роботов_after также следовал. Бандер
-  жестом отогнал других перед_роботов_after и вошел сам. 
Выделение для title:
- Книги before_один_after

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
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер жестом отогнал других роботов и вошел сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u'Они следовали за Бандером. Роботы before_остались_after на вежливом расстоянии, ',
                                                u' троих в комнату. before_Один_after из перед_роботов_after также следовал. Бандер',
                                                u' жестом отогнал других перед_роботов_after и вошел сам. '],
                                   u'title': [u'Книги before_один_after']}}],
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
`no_match_size` функционирует аналогично опции `allow_empty`. Если установлено значение 0, это работает как `allow_empty=1`, позволяя возвращать пустую строку в качестве результата подсветки, когда сниппет не может быть сгенерирован. В противном случае будет возвращено начало поля. Это необязательный параметр, значение по умолчанию равно 1.

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
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер жестом послал других роботов прочь и сам вошел. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u'Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ',
                                                u' три в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер',
                                                u' жестом послал других <b>роботов</b> прочь и сам вошел. Дверь'],
                                   u'title': [u'Книги <b>один</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер жестом послал других роботов прочь и сам вошел. Дверь закрылась за ним. "},"highlight":{"content":["Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, "," три в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер"," жестом послал других <b>роботов</b> прочь и сам вошел. Дверь"],"title":["Книги <b>один</b>"]}}]}}

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
title : Книги один
content : Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер жестом послал других роботов прочь и сам вошел. Дверь закрылась за ним.
Highlight for content:
-  три в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер
-  жестко отстранил других <b>роботов</b> и вошёл сам. Дверь
- Они следовали за Бандером. <b>Роботы</b> оставались на уважительном расстоянии,
Подсветка для заголовка:
- Книги <b>один</b>
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
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на уважительном расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер ввёл всех троих в комнату. Один из роботов также следовал. Бандер жестом отстранил других роботов и вошёл сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u' троих в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер',
                                                u' жестом отстранил других <b>роботов</b> и вошёл сам. Дверь',
                                                u'Они следовали за Бандером. <b>Роботы</b> оставались на уважительном расстоянии, '],
                                   u'title': [u'Книги <b>один</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы оставались на уважительном расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер ввёл всех троих в комнату. Один из роботов также следовал. Бандер жестом отстранил других роботов и вошёл сам. Дверь закрылась за ним. "},"highlight":{"content":[" троих в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер"," жестом отстранил других <b>роботов</b> и вошёл сам. Дверь","Они следовали за Бандером. <b>Роботы</b> оставались на уважительном расстоянии, "],"title":["Книги <b>один</b>"]}}]}}


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
      *: 'Текст 1'
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
				"content":"Текст 1",
				"name":"Документ 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1"};
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
				"content":"Текст 1",
				"name":"Документ 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		]}
	}
}
```


<!-- end -->

<!-- example fragment_size -->
#### fragment_size
`fragment_size` устанавливает максимальный размер фрагмента в символах. Это может быть глобальное значение или значение для конкретного поля. Параметры для конкретных полей переопределяют глобальные параметры. Это необязательный параметр, по умолчанию равный 256. Он работает аналогично параметру `limit`.

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
    echo 'Документ: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Подсветка для ".$field.":
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
Документ: 1
заголовок : Книги один
содержимое : Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер провёл всех троих в комнату. Один из роботов тоже последовал за ними. Бандер жестом отогнал других роботов и зашёл сам. Дверь закрылась за ним.
Выделить для содержимого:
-  комнату. <b>Один</b> из <b>роботов</b> тоже последовал за ними
- Бандер жестом отогнал других <b>роботов</b> и вошёл
Выделить для заголовка:
- Книги <b>один</b>
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
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер провёл всех троих в комнату. Один из роботов тоже последовал за ними. Бандер жестом отогнал других роботов и зашёл сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u' комнату. <b>Один</b> из <b>роботов</b> тоже последовал за ними',
                                                u'Бандер жестом отогнал других <b>роботов</b> и вошёл '],
                                   u'title': [u'Книги <b>один</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер провёл всех троих в комнату. Один из роботов тоже последовал за ними. Бандер жестом отогнал других роботов и зашёл сам. Дверь закрылась за ним. "},"highlight":{"content":[" комнату. <b>Один</b> из <b>роботов</b> тоже последовал за ними","Бандер жестом отогнал других <b>роботов</b> и вошёл "],"title":["Книги <b>один</b>"]}}]}}
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
      *: 'Текст 1'
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1"};
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example number_of_fragments -->
#### number_of_fragments
`number_of_fragments` ограничивает максимальное количество фрагментов в результате. Как `fragment_size`, он может быть глобальным или заданным для конкретного поля. Это необязательный параметр, по умолчанию равный 0 (без ограничений). Он работает аналогично опции `limit_snippets`.

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
    echo 'Документ: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Выделение для ".$field.":
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
content : Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие всегда ощущалось как угроза. Бандер ввел всех троих в комнату. Один из роботов тоже следовал за ними. Бандер указал другим роботам уйти и вошел сам. Дверь закрылась за ним.
Highlight for content:
- Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии,
-  троих в комнату. <b>Один</b> из <b>роботов</b> тоже следовал за ними. Бандер
-  указал другим <b>роботам</b> уйти и вошел сам. The
Highlight for title:
- Книги <b>один</b>

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
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие всегда ощущалось как угроза. Бандер ввел всех троих в комнату. Один из роботов тоже следовал за ними. Бандер указал другим роботам уйти и вошел сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u'Они следовали за Бандером. The <b>роботы</b> оставались на вежливом расстоянии, ',
                                                u' троих в комнату. <b>Один</b> из <b>роботов</b> тоже следовал за ними. Бандер',
                                                u' указал другим <b>роботам</b> уйти и вошел сам. The'],
                                   u'title': [u'Книги <b>один</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие всегда ощущалось как угроза. Бандер ввел всех троих в комнату. Один из роботов тоже следовал за ними. Бандер указал другим роботам уйти и вошел сам. Дверь закрылась за ним. "},"highlight":{"content":["Они следовали за Бандером. The <b>роботы</b> оставались на вежливом расстоянии, "," троих в комнату. <b>Один</b> из <b>роботов</b> тоже следовал за ними. Бандер"," указал другим <b>роботам</b> уйти и вошел сам. The"],"title":["Книги <b>один</b>"]}}]}}

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
      *: 'Текст 1'
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1"};
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example highlight json per-field limits -->

#### limit, limit_words, limit_snippets
Опции, такие как `limit`, `limit_words` и `limit_snippets`, могут быть установлены как глобальные или пер-полям. Глобальные опции используются как ограничения по полям, если пер-полям-опции не переопределяют их. В примере поле `title` выделено с настройками по умолчанию, в то время как поле `content` использует другое ограничение.
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
title : Книги один
content : Они следовали за Бандером. Роботы остались на вежливом расстоянии, но их присутствие было постоянно ощущаемой угрозой. Бандер провел всех троих в комнату. Один из роботов последовал тоже. Бандер жестом отослал остальных роботов и вошел сам. Дверь закрылась за ним.
Highlight for content:
-  в комнату. <b>Один</b> из <b>роботов</b> последовал тоже
Highlight for title:
- Книги <b>один</b>
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
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы остались на вежливом расстоянии, но их присутствие было постоянно ощущаемой угрозой. Бандер провел всех троих в комнату. Один из роботов последовал тоже. Бандер жестом отослал остальных роботов и вошел сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u' в комнату. <b>Один</b> из <b>роботов</b> последовал тоже'],
                                   u'title': [u'Книги <b>один</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы остались на вежливом расстоянии, но их присутствие было постоянно ощущаемой угрозой. Бандер провел всех троих в комнату. Один из роботов последовал тоже. Бандер жестом отослал остальных роботов и вошел сам. Дверь закрылась за ним. "},"highlight":{"title":["Книги <b>один</b>"],"content":[" в комнату. <b>Один</b> из <b>роботов</b> последовал тоже"]}}]}}
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
					"<b>Текст</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example highlight json global limits -->

#### limits_per_field
Глобальные ограничения также могут быть применены путем указания `limits_per_field=0`. Установка этой опции означает, что все комбинированные результаты выделения должны находиться в указанных пределах. Недостатком является то, что вы можете получить несколько фрагментов, выделенных в одном поле, и ни в одном другом, если движок выделения решит, что они более актуальны.

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
$bool = new ManticoresearchQueryBoolQuery();
$bool->must(new ManticoresearchQueryMatch(['query' => 'and first'], 'content'));

$results = $index->search($bool)->highlight(['content'=>['limit'=>50]],['limits_per_field'=>false])->get();
foreach($results as $doc)
{
    echo 'Документ: '.$doc->getId()."
";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."
";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Выделение для ".$field.":
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
Документ: 1
title : Книги один
content : Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер ввел всех троих в комнату. Один из роботов последовал за ними. Бандер жестом отослал других роботов и вошел сам. Дверь закрылась за ним.
Выделение для content:
-  жестом отослал других роботов <b>и</b> вошел сам. Дверь закрылась
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
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер ввел всех троих в комнату. Один из роботов последовал за ними. Бандер жестом отослал других роботов и вошел сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u' жестом отослал других роботов <b>и</b> вошел сам. Дверь закрылась']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1597,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер ввел всех троих в комнату. Один из роботов последовал за ними. Бандер жестом отослал других роботов и вошел сам. Дверь закрылась за ним. "},"highlight":{"content":[" жестом отослал других роботов <b>и</b> вошел сам. Дверь закрылась"]}}]}}

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
      *: 'Текст 1'
    }
  },
  highlight: { limits_per_field: 0 }
});
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlight.SetLimitsPerField(0)
searchRequest.SetHighlight(highlight)
# Подсветка

<!-- example highlighting -->

Подсветка позволяет получать фрагменты текста с подсветкой (называемые сниппетами) из документов, содержащих совпадающие ключевые слова.

Функция SQL `HIGHLIGHT()`, свойство `"highlight"` в JSON-запросах через HTTP и функция `highlight()` в PHP-клиенте все используют встроенное хранилище документов для получения оригинального содержимого полей (включено по умолчанию).

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
| Не <b>пробуй</b> состязаться в детских поступках, сказал Блис. |
+----------------------------------------------------------+
1 строка в наборе (0.00 сек)
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
          "title":"Книга четыре",
          "content":"Не пробуй состязаться в детских поступках, сказал Блис."
        },
        "highlight":
        {
          "title": ["Книга четыре"],
          "content": ["Не <b>пробуй</b> состязаться в детских поступках, сказал Блис."]
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
    echo 'Документ: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Подсветка для ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}

```

<!-- response PHP -->
```php
Документ: 14
title: Книга четыре
content: Не пробуй состязаться в детских поступках, сказал Блис.
Подсветка для title:
- Книга четыре
Подсветка для content:
- Не <b>пробуй</b> состязаться в детских поступках, сказал Блис.

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
                    u'_source': {u'content': u'Не пробуй состязаться в детских поступках, сказал Блис.',
                                 u'title': u'Книга четыре'},
                    u'highlight': {u'content': [u'Не <b>пробуй</b> состязаться в детских поступках, сказал Блис.'],
                                   u'title': [u'Книга четыре']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 4,"_score":1695,"_source":{"title":"Книга четыре","content":"Не пробуй состязаться в детских поступках, сказал Блис."},"highlight":{"title":["Книга четыре"],"content":["Не <b>пробуй</b> состязаться в детских поступках, сказал Блис."]}}]}}
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
        hits: [{_id=3, _score=1597, _source={title=Книга три, content=Тревиз шепнул: "Это вызывает инфантильное удовольствие от представления. Я бы с удовольствием сбил это."}, highlight={title=[Книга три], content=[, "Это <b>вызывает</b> инфантильное удовольствие ,  чтобы сбить это <b>вниз</b>."]}}, {_id=4, _score=1563, _source={title=Книга четыре, content=Не пробуй состязаться в детских поступках, сказал Блис.}, highlight={title=[Книга четыре], content=[Не <b>пробуй</b> состязаться в детских поступках, <b>сказал</b> Блис.]}}, {_id=5, _score=1514, _source={title=Книги две, content=Дверь открылась перед ними, открывая небольшую комнату. Бандер сказал: "Идите, полухуманы, я хочу показать вам, как мы живем."}, highlight={title=[Книги две], content=[ небольшая комната. Бандер <b>сказал</b>, "Идите, полухуманы, я]}}]
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
        hits: [{_id=3, _score=1597, _source={title=Третья книга, content=Тревиз шептал: "Оно получает инфантильное удовольствие от демонстрации. Я бы с удовольствием его сбил."}, highlight={title=[Третья книга], content=[, "Оно <b>получает</b> инфантильное удовольствие,  чтобы сбить его <b>вниз</b>."]}}, {_id=4, _score=1563, _source={title=Четвёртая книга, content=Не пытайся соревноваться в детскости, сказала Блисс.}, highlight={title=[Четвёртая книга], content=[Не <b>пытайся</b> соревноваться в детскости, <b>сказала</b> Блисс.]}}, {_id=5, _score=1514, _source={title=Книги две, content=Дверь открылась перед ними, открывая маленькую комнату. Бандер сказал: "Идите, полу-люди, я хочу показать вам, как мы живем."}, highlight={title=[Книги две], content=[ маленькую комнату. Бандер <b>сказал</b>, "Идите, полу-люди, я]}}]
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

При использовании SQL для выделения результатов поиска, вы получите фрагменты из различных полей, объединенные в одну строку из-за ограничений протокола MySQL. Вы можете настроить разделители для объединения с помощью опций `field_separator` и `snippet_separator`, как указано ниже.

При выполнении JSON-запросов через HTTP или использовании PHP-клиента таких ограничений нет и набор результатов включает массив полей, содержащих массивы фрагментов (без разделителей).

Учтите, что параметры генерации фрагментов, такие как `limit`, `limit_words` и `limit_snippets`, по умолчанию применяются к каждому полю отдельно. Вы можете изменить это поведение, используя параметр `limits_per_field`, но это может привести к нежелательным результатам. Например, одно поле может содержать ключевые слова, соответствующие запросу, но фрагменты из этого поля не будут включены в результирующий набор, поскольку они не получили такой высокий рейтинг, как фрагменты из других полей в механизме выделения.

Алгоритм выделения в настоящее время отдает предпочтение более качественным фрагментам (с более точными совпадениями фраз), а затем фрагментам с ключевыми словами, которые еще не включены в результат. Основная цель — выделить наилучшее соответствие запросу и выделить все ключевые слова запроса, насколько позволяют ограничения. Если соответствия в текущем поле не найдены, начало документа будет обрезано в соответствии с ограничениями и возвращено по умолчанию. Чтобы вернуть пустую строку, установите параметр `allow_empty` в значение 1.

Выделение выполняется на так называемом этапе `post limit`, что означает, что генерация фрагментов откладывается не только до тех пор, пока не будет подготовлен окончательный набор результатов, но и после применения операторов LIMIT. Например, в случае с оператором LIMIT 20,10 функция `HIGHLIGHT()` будет вызвана максимум 10 раз.

## Опции выделения

<!-- пример опций выделения -->

Существует несколько дополнительных опций выделения, которые можно использовать для точной настройки генерации фрагментов, и которые являются общими для SQL, HTTP и PHP клиентов.

#### before_match
Строка, вставляемая перед совпадением ключевого слова. В этой строке можно использовать макрос `%SNIPPET_ID%`. Первое вхождение макроса заменяется на инкрементирующийся номер фрагмента в пределах текущего фрагмента. Нумерация начинается с 1 по умолчанию, но может быть переопределена с помощью опции `start_snippet_id`. %SNIPPET_ID% начинается заново в начале каждого нового документа. По умолчанию это `<b>`.

#### after_match
Строка, вставляемая после совпадения ключевого слова. По умолчанию это `</b>`.

#### limit
Максимальный размер фрагмента, в символах (кодовых точках). По умолчанию 256. Это применяется к каждому полю отдельно, см. `limits_per_field`.

#### limit_words
Ограничивает максимальное количество слов, которые могут быть включены в результат. Обратите внимание, что это ограничение применяется ко всем словам, а не только к совпадающим ключевым словам для выделения. Например, если выделяется `Mary`, и выбран фрагмент `Mary had a little lamb`, это даёт 5 слов по этому ограничению, а не только 1. По умолчанию 0 (без ограничений). Это применяется к каждому полю отдельно, см. `limits_per_field`.

#### limit_snippets
Ограничивает максимальное количество фрагментов, которые могут быть включены в результат. По умолчанию 0 (без ограничений). Это применяется к каждому полю отдельно, см. `limits_per_field`.

#### limits_per_field
Определяет, действуют ли `limit`, `limit_words` и `limit_snippets` как индивидуальные ограничения для каждого поля выделяемого документа или как глобальные ограничения для всего документа. Установка этой опции в значение 0 означает, что все комбинированные результаты выделения для одного документа должны укладываться в заданные ограничения. Недостатком является то, что в одном поле может быть выделено несколько фрагментов, а в другом ни одного, если механизм выделения решает, что они более релевантны. По умолчанию 1 (использовать ограничения per-field).
#### around
Количество слов, которое следует выбирать вокруг каждого блока совпадающих ключевых слов. По умолчанию 5.

#### use_boundaries
Определяет, следует ли дополнительно разбивать фрагменты по границам фраз, как настроено в параметрах таблицы с директивой [phrase_boundary](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary). Значение по умолчанию - 0 (не использовать границы).

#### вес_порядок
Указывает, следует ли сортировать извлеченные фрагменты по порядку важности (убывающий вес) или по порядку появления в документе (возрастающая позиция). Значение по умолчанию - 0 (не использовать порядок веса).

#### принудительные_все_слова
Игнорирует ограничение по длине, пока результат не включает все ключевые слова. Значение по умолчанию - 0 (не принуждать к использованию всех ключевых слов).

#### стартовый_id_фрагмента
Устанавливает начальное значение макроса `%SNIPPET_ID%` (который обнаруживается и расширяется в строках `before_match`, `after_match`). Значение по умолчанию - 1.

#### html_strip_mode
Определяет режим очистки HTML. Значение по умолчанию - `index`, что означает, что будут использоваться настройки таблицы. Другие значения включают `none` и `strip`, которые принудительно пропускают или применяют очистку, независимо от настроек таблицы; и `retain`, который сохраняет HTML-разметку и защищает ее от выделения. Режим `retain` может использоваться только при выделении полных документов, и, следовательно, требует, чтобы не устанавливались ограничения по размеру фрагментов. Допустимые строковые значения - `none`, `strip`, `index` и `retain`.

#### разрешить_пустое
Позволяет вернуть пустую строку в качестве результата выделения, когда в текущем поле не удалось сгенерировать фрагменты (нет совпадения ключевых слов или ни один фрагмент не соответствует лимиту). По умолчанию вместо пустой строки будет возвращено начало оригинального текста. Значение по умолчанию - 0 (не разрешать пустой результат).

#### граница_фрагмента
Обеспечивает, чтобы фрагменты не пересекали границу предложения, абзаца или зоны (при использовании с таблицей, в которой включены соответствующие индексирующие настройки). Допустимые значения - `sentence`, `paragraph` и `zone`.

#### выдать_зоны
Испускает HTML-тег с именем окружающей зоны перед каждым фрагментом. Значение по умолчанию - 0 (не испускать имена зон).

#### принудить_фрагменты
Определяет, следует ли принудительно генерировать фрагменты, даже если ограничения позволяют выделение всего текста. Значение по умолчанию - 0 (не принуждать к генерации фрагментов).

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
Не <b>пытайтесь</b> соревноваться в детскости, <b>сказал</b> Блисс.

Документ: 2
заголовок: Книга вторая
содержание: Дверь открылась перед ними, открывая маленькую комнату. Бандер сказал: "Идите, получеловеки, я хочу показать вам, как мы живем.
Выделение для заголовка:
- Книга вторая
Выделение для содержания:
 маленькая комната. Бандер <b>сказал</b>, \"Идите, получеловеки, я
```

<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"пытайтесь"}},"highlight":{"limit":50}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'4',
                    u'_score': 1695,
                    u'_source': {u'content': u'Не пытайтесь соревноваться в детскости, сказал Блисс.',
                                 u'title': u'Книга четвертая'},
                    u'highlight': {u'content': [u'Не <b>пытайтесь</b> соревноваться в детскости, сказал Блисс.'],
                                   u'title': [u'Книга четвертая']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"query_string":"пытайтесь|получает|сбивает|сказал"},"highlight":{"limit":50}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":3,"hits":[{"_id": 3,"_score":1597,"_source":{"title":"Книга третья","content":"Тревиз шепнул: \"Это получает infantile удовольствие от показа. Мне бы хотелось его сбить.\""},"highlight":{"title":["Книга третья"],"content":[", \"Это <b>получает</b> infantile удовольствие "," сбить его <b>сбить</b>.\""]}},{"_id": 4,"_score":1563,"_source":{"title":"Книга четвертая","content":"Не пытайтесь соревноваться в детскости, сказал Блисс."},"highlight":{"title":["Книга четвертая"],"content":["Не <b>пытайтесь</b> соревноваться в детскости, <b>сказал</b> Блисс."]}},{"_id": 5,"_score":1514,"_source":{"title":"Книги вторая","content":"Дверь открылась перед ними, открывая маленькую комнату. Бандер сказал: \"Идите, получеловеки, я хочу показать вам, как мы живем.\""},"highlight":{"title":["Книги вторая"],"content":[" маленькая комната. Бандер <b>сказал</b>, \"Идите, получеловеки, я"]}}]}}
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","пытайтесь|получает|сбивает|сказал");
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
        hits: [{_id=3, _score=1597, _source={title=Книга третья, content=Тревиз шепнул, "Это получает infantile удовольствие от показа. Мне бы хотелось его сбить."}, highlight={title=[Книга третья], content=[, "Это <b>получает</b> infantile удовольствие ,  сбить его <b>сбить</b>."]}}, {_id=4, _score=1563, _source={title=Книга четвертая, content=Не пытайтесь соревноваться в детскости, сказал Блисс.}, highlight={title=[Книга четвертая], content=[Не <b>пытайтесь</b> соревноваться в детскости, <b>сказал</b> Блисс.]}}, {_id=5, _score=1514, _source={title=Книги вторая, content=Дверь открылась перед ними, открывая маленькую комнату. Бандер сказал: "Идите, получеловеки, я хочу показать вам, как мы живем."}, highlight={title=[Книги вторая], content=[ маленькая комната. Бандер <b>сказал</b>, "Идите, получеловеки, я]}}]
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
searchRequest.FulltextFilter = new MatchFilter("*", "пытайтесь|получает|сбивает|сказал");
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
        hits: [{_id=3, _score=1597, _source={title=Книга третья, content=Тревиз шепнул, "Это получает infantile удовольствие от показа. Мне бы хотелось его сбить."}, highlight={title=[Книга третья], content=[, "Это <b>получает</b> infantile удовольствие ,  сбить его <b>сбить</b>."]}}, {_id=4, _score=1563, _source={title=Книга четвертая, content=Не пытайтесь соревноваться в детскости, сказал Блисс.}, highlight={title=[Книга четвертая], content=[Не <b>пытайтесь</b> соревноваться в детскости, <b>сказал</b> Блисс.]}}, {_id=5, _score=1514, _source={title=Книги вторая, content=Дверь открылась перед ними, открывая маленькую комнату. Бандер сказал: "Идите, получеловеки, я хочу показать вам, как мы живем."}, highlight={title=[Книги вторая], content=[ маленькая комната. Бандер <b>сказал</b>, "Идите, получеловеки, я]}}]
        aggregations: null
    }
    profile: null
}
```
<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: { match: { *: 'Текст } },
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		},
		{
			"_id": 2,
			"_score":1480,
			"_source":
			{
				"content":"Текст 2",
				"name":"Док 2",
				"cat":2
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 2</b>"
				]
			}
		}]
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1"};
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
				"content":"Текст 1",
				"name":"Документ 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		},
		{
			"_id": 2,
			"_score":1480,
			"_source":
			{
				"content":"Текст 2",
				"name":"Документ 2",
				"cat":2
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 2</b>"
				]
			}
		}]
	}
}
```


<!-- end -->


## Подсветка через SQL

Функция `HIGHLIGHT()` может быть использована для подсветки результатов поиска. Синтаксис:

```sql
HIGHLIGHT([options], [field_list], [query] )
```

<!-- example highlight() no args -->
По умолчанию она работает без аргументов.

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
| Дверь <b>открылась</b> перед ними, открывая небольшую комнату. |
+-----------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() field syntax -->

`HIGHLIGHT()` извлекает все доступные полнотекстовые поля из хранения документов и подсвечивает их в соответствии с предоставленным запросом. Поддерживается синтаксис полей в запросах. Текст полей разделяется `field_separator`, который можно изменить в параметрах.

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
| Книга <b>один</b> |
+-----------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() options -->
Обязательный первый аргумент в `HIGHLIGHT()` - это список параметров.

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
| Книга [match]один[/match]                                    |
+------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() field list -->

Необязательный второй аргумент - это строка, содержащая одно поле или разделенный запятой список полей. Если этот аргумент присутствует, из хранения документов будет извлечены только указанные поля и подсвечены. Пустая строка в качестве второго аргумента обозначает "извлечь все доступные поля."

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
| Книга <b>один</b> | Они следовали за Бандером. <b>Роботы</b> оставались на вежлом расстоянии, но их присутствие было постоянно ощутимой угрозой.                                             |
| Бандер провел всех троих в комнату. <b>Один</b> из <b>роботов</b> следовал за ним. Бандер жестом велел другим <b>роботам</b> отойти и вошел сам. Дверь закрылась за ним. |
+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() string attr -->

Альтернативно, вы можете использовать второй аргумент, чтобы указать строковый атрибут или имя поля без кавычек. В этом случае переданная строка будет подсвечена в соответствии с предоставленным запросом, но синтаксис полей будет проигнорирован.

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
| Книга <b>один</b>     |
| Книга пять           |
+---------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example highlight() query -->

Необязательный третий аргумент - это запрос. Он используется для подсветки результатов поиска в соответствии с запросом, отличным от того, который использовался для поиска.

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
| Книга один                     |
| Книга <b>пять</b>              |
+-------------------------------+
2 строки в наборе (0.00 сек)
```

<!-- end -->

<!-- example HIGHLIGHT TO_STRING -->

Хотя `HIGHLIGHT()` предназначена для работы с хранимыми полнотекстовыми полями и строковыми атрибутами, её также можно использовать для выделения произвольного текста. Имейте в виду, что если запрос содержит любые операторы поиска по полям (например, `@title hello @body world`), часть поля игнорируется в этом случае.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT HIGHLIGHT({},TO_STRING('некоторый текст для выделения'), 'highlight') FROM books WHERE MATCH('@title one');
```

<!-- response SQL -->
```sql
+----------------------------------------------------------------+
| highlight({},TO_STRING('некоторый текст для выделения'), 'highlight') |
+----------------------------------------------------------------+
| некоторый текст для <b>выделения</b>                                  |
+----------------------------------------------------------------+
1 строка в наборе (0.00 сек)
```

<!-- end -->

Несколько опций актуальны только при генерации одной строки в качестве результата (не массива фрагментов). Это касается исключительно функции SQL `HIGHLIGHT()`:

#### snippet_separator
Строка для вставки между фрагментами. По умолчанию это ` ... `.
#### field_separator
Строка для вставки между полями. По умолчанию это `|`.


Другой способ выделения текста — это использование оператора [CALL SNIPPETS](../Searching/Highlighting.md#CALL-SNIPPETS). Это в основном дублирует функциональность `HIGHLIGHT()`, но не может использовать встроенное хранилище документов. Тем не менее, он может загружать исходный текст из файлов.


## Выделение через HTTP

<!-- example highlight in JSON -->

Чтобы выделять результаты полнотекстового поиска в JSON-запросах через HTTP, содержимое полей должно храниться в хранилище документов (включено по умолчанию). В примере полнотекстовые поля `content` и `title` извлекаются из хранилища документов и выделяются согласно запросу, указанному в условии `query`.

Выделенные фрагменты возвращаются в свойстве `highlight` массива `hits`.

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
          "title": "Книги один",
          "content": "Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал. Бандер жестом указал другим роботам подождать и вошёл сам. Дверь закрылась за ним."
        },
        "highlight": {
          "content": [
            "Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ",
            " три в комнату. <b>Один</b> из <b>роботов</b> тоже последовал. Бандер",
            " указал другим <b>роботам</b> подождать и вошёл сам. Дверь"
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
    echo 'Документ: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Выделение для ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}

```

<!-- response PHP -->
```php
Документ: 1
title : Книги один
content : Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал. Бандер жестом указал другим роботам подождать и вошёл сам. Дверь закрылась за ним.
Выделение для content:
- Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии,
-  три в комнату. <b>Один</b> из <b>роботов</b> тоже последовал. Бандер
-  указал другим <b>роботам</b> подождать и вошёл сам. The
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
                    u'_source': {u'content': u'Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал. Бандер жестом указал другим роботам подождать и вошёл сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u'Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ',
                                                u' три в комнату. <b>Один</b> из <b>роботов</b> тоже последовал. Бандер',
                                                u' указал другим <b>роботам</b> подождать и вошёл сам. The']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал. Бандер пригласил других роботов уйти и вошел сам. Дверь закрылась за ним. "},"highlight":{"content":["Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, "," троих в комнату. <b>Один</b> из <b>роботов</b> тоже последовал. Бандер"," пригласил остальных <b>роботов</b> уйти и вошел сам. Дверь"]}}]}}
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
        hits: [{_id=1, _score=2788, _source={title=Книги один, content=Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал. Бандер пригласил других роботов уйти и вошел сам. Дверь закрылась за ним. }, highlight={title=[Книги <b>один</b>], content=[Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ,  троих в комнату. <b>Один</b> из <b>роботов</b> тоже последовал. Бандер,  пригласил остальных <b>роботов</b> уйти и вошел сам. Дверь]} }]
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
        hits: [{_id=1, _score=2788, _source={title=Книги один, content=Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал. Бандер пригласил других роботов уйти и вошел сам. Дверь закрылась за ним. }, highlight={title=[Книги <b>один</b>], content=[Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ,  троих в комнату. <b>Один</b> из <b>роботов</b> тоже последовал. Бандер,  пригласил остальных <b>роботов</b> уйти и вошел сам. Дверь]} }]
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
      *: 'Текст 1|Текст 9'
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1|Текст 9"};
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- example highlight JSON all field  -->

Чтобы выделить все возможные поля, передайте пустой объект в качестве свойства `highlight`.

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
          "title": "Книги один",
          "content": "Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов тоже последовал. Бандер пригласил других роботов уйти и вошел сам. Дверь закрылась за ним. "
        },
        "highlight": {
          "title": [
            "Книги <b>один</b>"
          ],
          "content": [
            "Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ",
            " троих в комнату. <b>Один</b> из <b>роботов</b> тоже последовал. Бандер",
            " пригласил остальных <b>роботов</b> уйти и вошел сам. Дверь"
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
    echo 'Документ: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Выделение для ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}

```

<!-- response PHP -->
```php
Документ: 1
title : Книги один
content : Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянно ощущаемой угрозой. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер показал другим роботам прочь и сам вошел. Дверь закрылась за ним.
Выделение для title:
- Книги <b>один</b>
Выделение для content:
- Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии,
-  троих в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер
-  показал другим <b>роботам</b> прочь и сам вошел. Дверь

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
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянно ощущаемой угрозой. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер показал другим роботам прочь и сам вошел. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u'Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ',
                                                u' троих в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер',
                                                u' показал другим <b>роботам</b> прочь и сам вошел. Дверь'],
                                   u'title': [u'Книги <b>один</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянно ощущаемой угрозой. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер показал другим роботам прочь и сам вошел. Дверь закрылась за ним. "},"highlight":{"title":["Книги <b>один</b>"],"content":["Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, "," троих в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер"," показал другим <b>роботам</b> прочь и сам вошел. Дверь"]}}]}}

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
        hits: [{_id=1, _score=2788, _source={title=Книги один, content=Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянно ощущаемой угрозой. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер показал другим роботам прочь и сам вошел. Дверь закрылась за ним. }, highlight={title=[Книги <b>один</b>], content=[Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ,  троих в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер,  показал другим <b>роботам</b> прочь и сам вошел. Дверь]}}]
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
        hits: [{_id=1, _score=2788, _source={title=Книги один, content=Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянно ощущаемой угрозой. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер показал другим роботам прочь и сам вошел. Дверь закрылась за ним. }, highlight={title=[Книги <b>один</b>], content=[Они следовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ,  троих в комнату. <b>Один</b> из <b>роботов</b> также следовал. Бандер,  показал другим <b>роботам</b> прочь и сам вошел. Дверь]}}]
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
      *: 'Текст 1|Документ 1'
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
				"content":"Текст 1",
				"name":"Документ 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				],
				"name":
				[
					"<b>Документ 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1|Документ 1"};
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
				"content":"Текст 1",
				"name":"Документ 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст 1</b>"
				],
				"name":
				[
					"<b>Документ 1</b>"
				]
			}
		]}
	}
}
```


<!-- end -->

В дополнение к общим параметрам выделения, несколько синонимов доступны для JSON-запросов через HTTP:

#### fields
Объект `fields` содержит имена атрибутов с параметрами. Он также может быть массивом имен полей (без каких-либо параметров).

Обратите внимание, что по умолчанию выделение пытается выделить результаты в соответствии с запросом полного текста. В общем случае, когда вы не указываете поля для выделения, выделение основано на вашем запросе полного текста. Однако, если вы укажете поля для выделения, выделение выполняется только в том случае, если запрос полного текста совпадает с выбранными полями.

#### encoder
Кодировщик `encoder` может быть установлен на `default` или `html`. При установке на `html` он сохраняет HTML-разметку при выделении. Это работает аналогично опции `html_strip_mode=retain`.

<!-- example highlight_query -->
#### highlight_query
Параметр `highlight_query` позволяет вам выделять по запросу, отличному от вашего поискового запроса. Синтаксис такой же, как в основном `query`.

<!-- intro -->
##### JSON:
<!-- request JSON -->

```json
POST /search
{
  "table": "books",
  "query": { "match": { "content": "один|роботы" } },
  "highlight":
  {
    "fields": [ "content"],
    "highlight_query": { "match": { "*":"вежливое расстояние" } }
   }
}
```
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('books');
$bool = new \Manticoresearch\Query\BoolQuery();
$bool->must(new \Manticoresearch\Query\Match(['query' => 'один|роботы'], 'content'));

$results = $index->search($bool)->highlight(['content'],['highlight_query'=>['match'=>['*'=>'вежливое расстояние']]])->get();
foreach($results as $doc)
{
    echo 'Документ: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Выделение для ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```

<!-- request Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"content":"один|роботы"}},"highlight":{"fields":["content"],"highlight_query":{"match":{"*":"вежливое расстояние"}}}})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1788,
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер проводил всех троих в комнату. Один из роботов также следовал за ним. Бандер жестом отослал остальных роботов и вошел сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u'. Роботы оставались на <b>вежливом расстоянии</b>, но их присутствие было']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"content":"один|роботы"}},"highlight":{"fields":["content"],"highlight_query":{"match":{"*":"вежливое расстояние"}}}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1788,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер проводил всех троих в комнату. Один из роботов также следовал за ним. Бандер жестом отослал остальных роботов и вошел сам. Дверь закрылась за ним. "},"highlight":{"content":[". Роботы оставались на <b>вежливом расстоянии</b>, но их присутствие было"]}}]}}
```


<!-- intro -->
##### Java:

<!-- request Java -->

```java
searchRequest = new SearchRequest();
searchRequest.setIndex("books");
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","один|роботы");
}});        
searchRequest.setQuery(query);
highlight = new HashMap<String,Object>(){{
put("fields",new String[] {"content","title"});
put("highlight_query",
    new HashMap<String,Object>(){{
        put("match", new HashMap<String,Object>(){{
            put("*","вежливое расстояние");
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

#### pre_tags и post_tags
`pre_tags` и `post_tags` устанавливают открывающие и закрывающие теги для выделенных фрагментов текста. Они функционируют аналогично опциям `before_match` и `after_match`. Эти параметры являются необязательными, по умолчанию используются значения `<b>` и `</b>`.

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
content : Они следовали за Бандером. Роботы оставались на веждом расстоянии, но их присутствие было постоянно ощутимой угрозой. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер жестом отослал остальных роботов и вошел сам. Дверь закрылась за ним.
Highlight for content:
- Они следовали за Бандером. before_роботы_after оставались на веждом расстоянии,
-  троих в комнату. before_Один_after из before_роботов_after также следовал. Бандер
-  жестом отослал остальных before_роботов_after и вошел сам. 
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
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на веждом расстоянии, но их присутствие было постоянно ощутимой угрозой. Бандер провел всех троих в комнату. Один из роботов также следовал. Бандер жестом отослал остальных роботов и вошел сам. Дверь закрылась за ним. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'Они следовали за Бандером. before_роботы_after оставались на веждом расстоянии, ',
                                                u' троих в комнату. before_Один_after из before_роботов_after также следовал. Бандер',
                                                u' жестом отослал остальных before_роботов_after и вошел сам. ']},
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
`no_match_size` функционирует аналогично опции `allow_empty`. Если установлено значение 0, это действует как `allow_empty=1`, позволяя возвращать пустую строку в качестве результата подсветки, когда сниппет не может быть сгенерирован. В противном случае, будет возвращено начало поля. Это необязательный параметр, значение по умолчанию равно 1.

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
                    u'_source': {u'content': u'Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов также последовал за ним. Бандерgesturedawayandentereditself. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u'Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, ',
                                                u' троих в комнату. <b>Один</b> из <b>роботов</b> также последовал за ним. Бандер',
                                                u' жестом отогнал других <b>роботов</b> и вошел сам. Дверь'],
                                   u'title': [u'Книги <b>один</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов также последовал за ним. Бандерgesturedawayandentereditself. Дверь закрылась за ним. "},"highlight":{"content":["Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, "," троих в комнату. <b>Один</b> из <b>роботов</b> также последовал за ним. Бандер"," жестом отогнал других <b>роботов</b> и вошел сам. Дверь"],"title":["Книги <b>один</b>"]}}]}}

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
title : Книги один
content : Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провел всех троих в комнату. Один из роботов также последовал за ним. Бандерgesturedawayandentereditself. Дверь закрылась за ним.
Highlight for content:
-  троих в комнату. <b>Один</b> из <b>роботов</b> также последовал за ним. Бандер
- указал другим <b>роботам</b> уйти и вошёл сам. Дверь
- Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии,
Заголовок для выделения:
- Книги <b>один</b>
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
                    u'_source': {u'content': u'Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер проводил всех троих в комнату. Один из роботов тоже последовал за ним. Бандер указал другим роботам уйти и вошёл сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u' троих в комнату. <b>Один</b> из <b>роботов</b> тоже последовал за ним. Бандер',
                                                u' указал другим <b>роботам</b> уйти и вошёл сам. Дверь',
                                                u'Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, '],
                                   u'title': [u'Книги <b>один</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер проводил всех троих в комнату. Один из роботов тоже последовал за ним. Бандер указал другим роботам уйти и вошёл сам. Дверь закрылась за ним. "},"highlight":{"content":[" троих в комнату. <b>Один</b> из <b>роботов</b> тоже последовал за ним. Бандер"," указал другим <b>роботам</b> уйти и вошёл сам. Дверь","Они последовали за Бандером. <b>Роботы</b> оставались на вежливом расстоянии, "],"title":["Книги <b>один</b>"]}}]}}


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
`fragment_size` устанавливает максимальный размер фрагмента в символах. Он может быть глобальным или для каждого поля. Параметры для каждого поля переопределяют глобальные параметры. Это необязательно, с умолчательным значением 256. Работает аналогично параметру `limit`.

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
    echo 'Документ: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Выделение для ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```
<!-- response PHP -->
```php
Документ: 1
заголовок : Книги один
содержание : Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер ввел всех троих в комнату. Один из роботов последовал за ним. Бандер жестом отогнал остальных роботов и вошел сам. Дверь закрылась за ним.
Выделить содержание:
-  комнату. <b>Один</b> из <b>роботов</b> последовал за ним
- Бандер жестом отогнал остальных <b>роботов</b> и вошел
Выделить заголовок:
- Книги <b>один</b>
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
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер ввел всех троих в комнату. Один из роботов последовал за ним. Бандер жестом отогнал остальных роботов и вошел сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u' комнату. <b>Один</b> из <b>роботов</b> последовал за ним',
                                                u'Бандер жестом отогнал остальных <b>роботов</b> и вошел '],
                                   u'title': [u'Книги <b>один</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие постоянно ощущалось как угроза. Бандер ввел всех троих в комнату. Один из роботов последовал за ним. Бандер жестом отогнал остальных роботов и вошел сам. Дверь закрылась за ним. "},"highlight":{"content":[" комнату. <b>Один</b> из <b>роботов</b> последовал за ним","Бандер жестом отогнал остальных <b>роботов</b> и вошел "],"title":["Книги <b>один</b>"]}}]}}
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
      *: 'Текст 1'
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1"};
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
				"content":"Текст 1",
				"name":"Док 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст</b>"
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
$bool = new \Manticoresearch\Query\BoolQuery();
$bool->must(new \Manticoresearch\Query\Match(['query' => 'one|robots'], '*'));

$results = $index->search($bool)->highlight(['content','title'],['number_of_fragments'=>10])->get();
foreach($results as $doc)
{
    echo 'Документ: '.$doc->getId()."\n";
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

#### ограничение, limit_words, limit_snippets
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
content : Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянно ощущаемой угрозой. Бандер провел всех троих в комнату. Один из роботов также последовал за ними. Бандер жестом отослал других роботов и вошел сам. За ним закрылась дверь.
Highlight for content:
-  в комнату. <b>Один</b> из <b>роботов</b> также последовал за ними
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
                    u'_source': {u'content': u'Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянно ощущаемой угрозой. Бандер провел всех троих в комнату. Один из роботов также последовал за ними. Бандер жестом отослал других роботов и вошел сам. За ним закрылась дверь. ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u' в комнату. <b>Один</b> из <b>роботов</b> также последовал за ними'],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"Они последовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянно ощущаемой угрозой. Бандер провел всех троих в комнату. Один из роботов также последовал за ними. Бандер жестом отослал других роботов и вошел сам. За ним закрылась дверь. "},"highlight":{"title":["Books <b>one</b>"],"content":[" в комнату. <b>Один</b> из <b>роботов</b> также последовал за ними"]}}]}}
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
				"content":"Текст 1",
				"name":"Документ 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст</b>"
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
				"content":"Текст 1",
				"name":"Документ 1",
				"cat":1
			},
			"highlight":
			{
				"content":
				[
					"<b>Текст</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

<!-- пример ограничения глобальных лимитов json -->

#### limits_per_field
Глобальные ограничения также могут быть применены, указав `limits_per_field=0`. Установка этого параметра означает, что все объединённые результаты подсветки должны находиться в пределах заданных ограничений. Недостатком является то, что вы можете получить несколько фрагментов, выделенных в одном поле и ни одного в другом, если движок подсветки решит, что они более актуальны.

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
    echo 'Документ: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "Подсветка для ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```
<!-- response PHP -->
```php
Документ: 1
title : Книги один
content : Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провёл всех троих в комнату. Один из роботов тоже последовал за ними. Бандер указал другим роботам уйти и вошёл сам. Дверь закрылась за ним.
Подсветка для content:
-  указал другим роботам уйти <b>и</b> вошёл сам. Дверь закрылась
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
                    u'_source': {u'content': u'Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провёл всех троих в комнату. Один из роботов тоже последовал за ними. Бандер указал другим роботам уйти и вошёл сам. Дверь закрылась за ним. ',
                                 u'title': u'Книги один'},
                    u'highlight': {u'content': [u' указал другим роботам уйти <b>и</b> вошёл сам. Дверь закрылась']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1597,"_source":{"title":"Книги один","content":"Они следовали за Бандером. Роботы оставались на вежливом расстоянии, но их присутствие было постоянной угрозой. Бандер провёл всех троих в комнату. Один из роботов тоже последовал за ними. Бандер указал другим роботам уйти и вошёл сам. Дверь закрылась за ним. "},"highlight":{"content":[" указал другим роботам уйти <b>и</b> вошёл сам. Дверь закрылась"]}}]}}

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
      *: 'Текст 1'
    }
  },
  highlight: { limits_per_field: 0 }
});
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "Текст 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlight.SetLimitsPerField(0)
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- end -->

# СНИППЕТЫ ВЫЗОВА

<!-- пример СНИППЕТЫ ВЫЗОВА -->

Оператор `CALL SNIPPETS` создает фрагмент из предоставленных данных и запроса с использованием заданных настроек таблицы. Он не может получить доступ к встроенному хранилищу документов, поэтому рекомендуется использовать функцию [HIGHLIGHT()](../Searching/Highlighting.md) вместо.

Синтаксис:

```sql
CALL SNIPPETS(data, table, query[, opt_value AS opt_name[, ...]])
```

#### data
`data` служит источником, из которого извлекается фрагмент. Это может быть либо одна строка, либо список строк, заключенных в фигурные скобки.
#### table
`table` относится к имени таблицы, которая предоставляет настройки обработки текста для генерации фрагментов.
#### query
`query` — это полнотекстовый запрос, используемый для построения фрагментов.
#### opt_value и opt_name
`opt_value` и `opt_name` представляют собой [опции генерации фрагментов](../Searching/Highlighting.md).

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

Большинство опций такие же, как в функции [HIGHLIGHT()](../Searching/Highlighting.md). Однако есть несколько опций, которые можно использовать только с `CALL SNIPPETS`.

<!-- example CALL SNIPPETS load files -->
Следующие опции могут быть использованы для выделения текста, хранящегося в отдельных файлах:

#### load_files
Эта опция, когда включена, рассматривает первый аргумент как имена файлов, а не как данные для извлечения фрагментов. Указанные файлы на стороне сервера будут загружены для данных. Будет использовано до [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) рабочих потоков на запрос для параллелизации работы, когда этот флаг включен. По умолчанию 0 (без ограничения). Чтобы распределить генерацию фрагментов между удаленными агентами, вызовите генерацию фрагментов в распределенной таблице, содержащей только один(!) локальный агент и несколько удаленных. Опция [snippets_file_prefix](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#snippets_file_prefix) используется для генерации окончательного имени файла. Например, когда searchd настроен с `snippets_file_prefix = /var/data_` и `text.txt` указан как имя файла, фрагменты будут сгенерированы из содержимого `/var/data_text.txt`.

#### load_files_scattered
Эта опция работает только с распределенной генерацией фрагментов с удаленными агентами. Исходные файлы для генерации фрагментов могут быть распределены между различными агентами, и основной сервер объединит все неошибочные результаты. Например, если один агент распределенной таблицы имеет `file1.txt`, другой агент имеет `file2.txt`, и вы используете `CALL SNIPPETS` с обоими этими файлами, searchd объединит результаты агентов, так что вы получите результаты как из `file1.txt`, так и из `file2.txt`. По умолчанию 0.

Если опция `load_files` также включена, запрос вернет ошибку, если какой-либо из файлов недоступен. В противном случае (если `load_files` не включен) он вернет пустые строки для всех отсутствующих файлов. Searchd не передает этот флаг агентам, поэтому агенты не генерируют критическую ошибку, если файл не существует. Если вы хотите быть уверены, что все исходные файлы загружены, установите оба `load_files_scattered` и `load_files` в 1. Если отсутствие некоторых исходных файлов у некоторых агентов не критично, установите только `load_files_scattered` в 1.

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



