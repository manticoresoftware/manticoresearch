# हाइलाइटिंग

<!-- example highlighting -->

हाइलाइटिंग आपको दस्तावेज़ों से मेल खाते कीवर्ड वाली हाइलाइटेड टेक्स्ट फ्रेगमेंट (जिन्हें स्निप्पेट्स कहा जाता है) प्राप्त करने की अनुमति देती है।

SQL `HIGHLIGHT()` फ़ंक्शन, HTTP के जरिए JSON क्वेरी में `"highlight"` प्रॉपर्टी, और PHP क्लाइंट में `highlight()` फ़ंक्शन सभी मूल फ़ील्ड सामग्री को पुनः प्राप्त करने के लिए अंतर्निहित दस्तावेज़ स्टोरेज का उपयोग करते हैं (डिफ़ॉल्ट रूप से सक्षम)।

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
        hits: [{_id=3, _score=1597, _source={title=पुस्तक तीन, content=ट्रेवीज़ ने फुसफुसाया, "यह प्रदर्शन से बच्चों जैसा आनंद उठाता है। मैं इसे गिराना चाहूंगा।"}, highlight={title=[पुस्तक तीन], content=[, "यह <b>उठाता है</b> बच्चों जैसा आनंद ,  इसे <b>गिराना</b> चाहूंगा।"]}}, {_id=4, _score=1563, _source={title=पुस्तक चार, content=ब्लीस ने कहा, "बचपने में प्रतिस्पर्धा करने की कोशिश मत करो।"}, highlight={title=[पुस्तक चार], content=[मत <b>करो</b> बचपने में प्रतिस्पर्धा, <b>कहा</b> ब्लीस।]}}, {_id=5, _score=1514, _source={title=पुस्तकें दो, content=उनके सामने एक दरवाजा खुला, एक छोटे से कमरे का अनावरण करते हुए। बैंडर ने कहा, "आओ, आधे-मनुष्य, मैं तुमसे दिखाना चाहता हूँ कि हम कैसे जीते हैं।"}, highlight={title=[पुस्तकें दो], content=[ एक छोटे से कमरे का। बैंडर <b>ने कहा</b>, "आओ, आधे-मनुष्य, मैं"]}}]
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
      *: 'पाठ 1'
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
				"content":"पाठ 1"
			},
			"highlight":
			{
				"content":
				[
					"<b>पाठ 1</b>"
				]
			}
		]}
	}
}
```

<!-- request Go -->
``` go
matchClause := map[string]interface{} {"*": "पाठ 1"};
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
				"content":"पाठ 1"
			},
			"highlight":
			{
				"content":
				[
					"<b>पाठ 1</b>"
				]
			}
		]}
	}
}
```

<!-- end -->

जब आप SQL का उपयोग करके खोज परिणामों को हाइलाइट करते हैं, तो आपको विभिन्न फ़ील्डों से स्निपेट एकल स्ट्रिंग में संयोजित मिलेंगे, जो MySQL प्रोटोकॉल की सीमाओं के कारण है। आप `field_separator` और `snippet_separator` विकल्पों के साथ संयोजन अलगावक को समायोजित कर सकते हैं, जैसा कि नीचे विस्तार से बताया गया है।

HTTP के माध्यम से JSON क्वेरी निष्पादित करते समय या PHP क्लाइंट का उपयोग करते समय, ऐसी कोई सीमाएँ नहीं हैं, और परिणाम सेट में स्निपेट के ऐरे को शामिल करने वाले फ़ील्डों का ऐरे होता है (बिना अलगावक के)।

ध्यान रखें कि स्निपेट जनरेशन विकल्प जैसे `limit`, `limit_words`, और `limit_snippets` डिफ़ॉल्ट रूप से प्रत्येक फ़ील्ड पर व्यक्तिगत रूप से लागू होते हैं। आप `limits_per_field` विकल्प का उपयोग करके इसे बदल सकते हैं, लेकिन इससे अनचाहे परिणाम मिल सकते हैं। उदाहरण के लिए, एक फ़ील्ड में मेल खाने वाले कीवर्ड हो सकते हैं, लेकिन उस फ़ील्ड से कोई स्निपेट परिणाम सेट में शामिल नहीं होता है क्योंकि उनका रैंकिंग अन्य फ़ील्डों के स्निपेट की तुलना में उच्च नहीं था।

हाइलाइटिंग एल्गोरिदम वर्तमान में बेहतर स्निपेट (करीबी वाक्यांश मेल के साथ) को प्राथमिकता देता है और फिर ऐसे स्निपेट को जो कीवर्ड अभी तक परिणाम में शामिल नहीं हैं। सामान्यत: इसका उद्देश्य क्वेरी के लिए सबसे अच्छा मेल हाइलाइट करना और सभी क्वेरी कीवर्ड को हाइलाइट करना है, जैसा कि सीमाओं द्वारा अनुमति है। यदि वर्तमान फ़ील्ड में कोई मेल नहीं पाया जाता है, तो दस्तावेज़ की शुरुआत को सीमाओं के अनुसार ट्रिम किया जाएगा और डिफ़ॉल्ट रूप से लौटाया जाएगा। इसके बजाए एक खाली स्ट्रिंग लौटाने के लिए, `allow_empty` विकल्प को 1 पर सेट करें।

हाइलाइटिंग तथाकथित `पोस्ट लिमिट` चरण के दौरान की जाती है, जिसका अर्थ है कि स्निपेट जनरेशन केवल तब तक स्थगित नहीं है जब तक कि पूरा अंतिम परिणाम सेट तैयार न हो, बल्कि LIMIT क्लॉज लागू होने के बाद भी होती है। उदाहरण के लिए, LIMIT 20,10 क्लॉज के साथ, `HIGHLIGHT()` फ़ंक्शन को अधिकतम 10 बार कॉल किया जाएगा।

## हाइलाइटिंग विकल्प

<!-- example highlighting options -->

कई वैकल्पिक हाइलाइटिंग विकल्प हैं जिन्हें स्निपेट जनरेशन को ठीक करने के लिए उपयोग किया जा सकता है, जो SQL, HTTP और PHP क्लाइंटों के लिए सामान्य हैं।

#### before_match
एक स्ट्रिंग जिसे कीवर्ड मैच से पहले डालना है। इस स्ट्रिंग में `%SNIPPET_ID%` मैक्रो का उपयोग किया जा सकता है। मैक्रो की पहली उपस्थिति को वर्तमान स्निपेट के भीतर एक बढ़ते स्निपेट संख्या के साथ प्रतिस्थापित किया जाता है। संख्या डिफ़ॉल्ट रूप से 1 से शुरू होती है लेकिन इसे `start_snippet_id` विकल्प से ओवरराइड किया जा सकता है। %SNIPPET_ID% प्रत्येक नए दस्तावेज़ के शुरू में फिर से शुरू होता है। डिफ़ॉल्ट `<b>` है।

#### after_match
एक स्ट्रिंग जिसे कीवर्ड मैच के बाद डालना है। डिफ़ॉल्ट `</b>` है।

#### limit
अधिकतम स्निपेट आकार, प्रतीकों (कोडपॉइंट्स) में। डिफ़ॉल्ट 256 है। यह डिफ़ॉल्ट रूप से प्रति-फ़ील्ड लागू होता है, देखें `limits_per_field`।

#### limit_words
परिणाम में शामिल किए जा सकने वाले अधिकतम शब्दों की संख्या को सीमित करता है। ध्यान दें कि यह सीमा सभी शब्दों पर लागू होती है, न कि केवल हाइलाइट करने के लिए मेल खाने वाले कीवर्ड पर। उदाहरण के लिए, यदि हाइलाइटिंग `मैरी` है और एक स्निपेट `मैरी के पास एक छोटा मेमना` को चुना जाता है, तो यह इस सीमा में 5 शब्दों का योगदान देता है, केवल 1 नहीं। डिफ़ॉल्ट 0 है (कोई सीमा नहीं)। यह डिफ़ॉल्ट रूप से प्रति-फ़ील्ड लागू होता है, देखें `limits_per_field`।

#### limit_snippets
परिणाम में शामिल किए जा सकने वाले अधिकतम स्निपेट की संख्या को सीमित करता है। डिफ़ॉल्ट 0 है (कोई सीमा नहीं)। यह डिफ़ॉल्ट रूप से प्रति-फ़ील्ड लागू होता है, देखें `limits_per_field`।

#### limits_per_field
यह निर्धारित करता है कि `limit`, `limit_words`, और `limit_snippets` महत्वपूर्ण सीमाएँ होंगी या नहीं, जिनका संचालन दस्तावेज़ के प्रत्येक फ़ील्ड में हाइलाइट किया गया है, या पूरी दस्तावेज़ के लिए वैश्विक सीमाएँ। इस विकल्प को 0 पर सेट करने का अर्थ है कि एक दस्तावेज़ के लिए सभी संयुक्त हाइलाइटिंग परिणाम निर्दिष्ट सीमाओं के भीतर होना चाहिए। इसका नकारात्मक पक्ष यह है कि आपके पास एक फ़ील्ड में कई स्निपेट हाइलाइट किए जा सकते हैं और दूसरे में कोई नहीं हो सकता है यदि हाइलाइटिंग इंजन यह तय करता है कि वे अधिक प्रासंगिक हैं। डिफ़ॉल्ट 1 है (प्रति-फ़ील्ड सीमाएँ प्रयोग करें)।
#### around
प्रत्येक मेल खाने वाले कीवर्ड ब्लॉक के चारों ओर चयनित शब्दों की संख्या। डिफ़ॉल्ट 5 है।

#### use_boundaries
वाक्य सीमा पात्रों द्वारा अतिरिक्त रूप से स्निप्पेट्स को तोड़ने का निर्धारण करता है, जैसा कि तालिका सेटिंग्स में [phrase_boundary](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary) निर्देश में कॉन्फ़िगर किया गया है। डिफ़ॉल्ट 0 है (सीमाओं का उपयोग न करें)।

#### weight_order
निर्दिष्ट करता है कि निकाले गए स्निप्पेट्स को प्रासंगिकता के क्रम (घटते वजन) में या दस्तावेज़ में उपस्थिति के क्रम (बढ़ती स्थिति) में सॉर्ट करना है। डिफ़ॉल्ट 0 है (वजन क्रम का उपयोग न करें)।

#### force_all_words
तब तक लंबाई सीमा की अनदेखी करता है जब तक परिणाम में सभी कीवर्ड शामिल नहीं होते। डिफ़ॉल्ट 0 है (सभी कीवर्ड को मजबूत न करें)।

#### start_snippet_id
`%SNIPPET_ID%` मैक्रो का प्रारंभिक मान सेट करता है (जो `before_match`, `after_match` स्ट्रिंग्स में पहचाना और विस्तारित किया जाता है)। डिफ़ॉल्ट 1 है।

#### html_strip_mode
HTML स्ट्रिपिंग मोड सेटिंग को परिभाषित करता है। डिफ़ॉल्ट `index` है, अर्थात् तालिका सेटिंग्स का उपयोग किया जाएगा। अन्य मानों में `none` और `strip` शामिल हैं, जो तालिका सेटिंग्स की परवाह किए बिना मजबूती से स्ट्रिपिंग छोड़ते या लागू करते हैं; और `retain`, जो HTML मार्कअप को बनाए रखता है और इसे हाइलाइटिंग से सुरक्षित करता है। `retain` मोड केवल पूर्ण दस्तावेजों को हाइलाइटिंग करते समय उपयोग किया जा सकता है और इसलिए इसमें कोई स्निप्पेट आकार सीमा सेट नहीं की जानी चाहिए। अनुमति प्राप्त स्ट्रिंग मान `none`, `strip`, `index`, और `retain` हैं।

#### allow_empty
वर्तमान क्षेत्र में कोई स्निप्पेट उत्पन्न नहीं हो पाने पर हाइलाइटिंग परिणाम के रूप में एक खाली स्ट्रिंग लौटाने की अनुमति देता है (कोई कीवर्ड मैच या कोई स्निप्पेट सीमा में फिट नहीं होता)। डिफ़ॉल्ट रूप से, खाली स्ट्रिंग के बजाय मूल पाठ की शुरुआत लौटाई जाएगी। डिफ़ॉल्ट 0 है (खाली परिणाम की अनुमति न दें)।

#### snippet_boundary
सुनिश्चित करता है कि स्निप्पेट एक वाक्य, पैराग्राफ, या जोन सीमा को पार न करें (जब एक तालिका के साथ उपयोग किया जाता है जिसमें संबंधित अनुक्रमण सेटिंग्स सक्षम हैं)। अनुमत मान `sentence`, `paragraph`, और `zone` हैं।

#### emit_zones
प्रत्येक स्निप्पेट से पहले संलग्न जोन नाम के साथ एक HTML टैग उत्पन्न करता है। डिफ़ॉल्ट 0 है (जोन नाम उत्पन्न न करें)।

#### force_snippets
निर्धारित करता है कि क्या स्निप्पेट उत्पन्न करने के लिए मजबूर करना है, भले ही सीमाएँ पूरे पाठ को हाइलाइट करने की अनुमति देती हों। डिफ़ॉल्ट 0 है (स्निप्पेट जनरेशन के लिए मजबूर न करें)।

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
title: पुस्तक दो
content: उनके सामने एक दरवाजा खुला, एक छोटे कमरे का उद्घाटन करते हुए। बैंडर ने कहा, "आओ, आधे मानवों, मैं तुमको दिखाना चाहता हूँ कि हम कैसे जीते हैं।"
Highlight for title:
- पुस्तक दो
Highlight for content:
 एक छोटे कमरे का उद्घाटन करते हुए। बैंडर <b>ने कहा</b>, \"आओ, आधे मानवों, मैं
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

<!-- request Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"query_string":"try|gets|down|said"},"highlight":{"limit":50}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":3,"hits":[{"_id": 3,"_score":1597,"_source":{"title":"Book three","content":"Trevize whispered, \"It gets infantile pleasure out of display. I`d love to knock it down.\""},"highlight":{"title":["Book three"],"content":[", \"It <b>gets</b> infantile pleasure "," to knock it <b>down</b>.\""]}},{"_id": 4,"_score":1563,"_source":{"title":"Book four","content":"Don`t try to compete in childishness, said Bliss."},"highlight":{"title":["Book four"],"content":["Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss."]}},{"_id": 5,"_score":1514,"_source":{"title":"Books two","content":"A door opened before them, revealing a small room. Bander said, \"Come, half-humans, I want to show you how we live.\""},"highlight":{"title":["Books two"],"content":[" एक छोटे कमरे का उद्घाटन करते हुए। बैंडर <b>ने कहा</b>, \"आओ, आधे मानवों, मैं"]}}]}}
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
        hits: [{_id=3, _score=1597, _source={title=Book three, content=Trevize whispered, "It gets infantile pleasure out of display. I`d love to knock it down."}, highlight={title=[Book three], content=[, "It <b>gets</b> infantile pleasure ,  to knock it <b>down</b>."]}}, {_id=4, _score=1563, _source={title=Book four, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[Book four], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=Books two, content=A door opened before them, revealing a small room. Bander said, "Come, half-humans, I want to show you how we live."}, highlight={title=[Books two], content=[ एक छोटे कमरे का उद्घाटन करते हुए। बैंडर <b>ने कहा</b>, "आओ, आधे मानवों, मैं"]}}]
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
        hits: [{_id=3, _score=1597, _source={title=Book three, content=Trevize whispered, "It gets infantile pleasure out of display. I`d love to knock it down."}, highlight={title=[Book three], content=[, "It <b>gets</b> infantile pleasure ,  to knock it <b>down</b>."]}}, {_id=4, _score=1563, _source={title=Book four, content=Don`t try to compete in childishness, said Bliss.}, highlight={title=[Book four], content=[Don`t <b>try</b> to compete in childishness, <b>said</b> Bliss.]}}, {_id=5, _score=1514, _source={title=Books two, content=A door opened before them, revealing a small room. Bander said, "Come, half-humans, I want to show you how we live."}, highlight={title=[Books two], content=[ एक छोटे कमरे का उद्घाटन करते हुए। बैंडर <b>ने कहा</b>, "आओ, आधे मानवों, मैं"]}}]
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


## SQL के माध्यम से हाइलाइटिंग

`HIGHLIGHT()` फ़ंक्शन का उपयोग खोज परिणामों को हाइलाइट करने के लिए किया जा सकता है। यहाँ वाक्य रचना है:

```sql
HIGHLIGHT([options], [field_list], [query] )
```

<!-- example highlight() no args -->
डिफ़ॉल्ट रूप से, यह बिना तर्कों के काम करता है।

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

`HIGHLIGHT()` दस्तावेज़ भंडारण से सभी उपलब्ध पूर्ण-본문 फ़ील्ड को प्रगट करता है और उन्हें प्रदान किए गए प्रश्न के खिलाफ हाइलाइट करता है। प्रश्नों में फ़ील्ड सिंटेक्स का समर्थन किया जाता है। फ़ील्ड टेक्स्ट `field_separator` द्वारा अलग किया जाता है, जिसे विकल्पों में संशोधित किया जा सकता है।

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
`HIGHLIGHT()` में वैकल्पिक पहला तर्क विकल्पों की सूची है।

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

वैकल्पिक दूसरा तर्क एक ऐसा स्ट्रिंग है जो एकल फ़ील्ड या फ़ील्ड की अल्पविराम से अलग की गई सूची को समाहित करता है। यदि यह तर्क मौजूद है, तो केवल निर्दिष्ट फ़ील्ड को दस्तावेज़ भंडारण से निकाला जाएगा और हाइलाइट किया जाएगा। दूसरे तर्क के रूप में एक खाली स्ट्रिंग "सभी उपलब्ध फ़ील्ड निकालें" का संकेत देती है।

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

वैकल्पिक रूप से, आप दूसरे तर्क का उपयोग एक स्ट्रिंग विशेषता या फ़ील्ड नाम निर्दिष्ट करने के लिए कर सकते हैं बिना उद्धरण के। इस मामले में, दी गई स्ट्रिंग प्रदान किए गए प्रश्न के खिलाफ हाइलाइट की जाएगी, लेकिन फ़ील्ड सिंटेक्स को दरकिनार किया जाएगा।

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

वैकल्पिक तीसरा तर्क प्रश्न है। इसका उपयोग खोज परिणामों को एक प्रश्न के खिलाफ हाइलाइट करने के लिए किया जाता है जो खोज के लिए उपयोग किए गए प्रश्न से भिन्न है।

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
| किताब <b>पाँच</b>                 |
+-------------------------------+
2 पंक्तियाँ सेट में (0.00 सेकंड)
```

<!-- अंत -->

<!-- उदाहरण HIGHLIGHT TO_STRING -->

हालाँकि `HIGHLIGHT()` को संग्रहीत पूर्ण-पाठ क्षेत्रों और स्ट्रिंग विशेषताओं के साथ काम करने के लिए डिजाइन किया गया है, इसे मनमाने टेक्स्ट को हाइलाइट करने के लिए भी उपयोग किया जा सकता है। ध्यान रखें कि यदि क्वेरी में कोई फ़ील्ड खोज ऑपरेटर होते हैं (जैसे `@title hello @body world`), तो इस मामले में उनके फ़ील्ड भाग की अनदेखी की जाती है।

<!-- परिचय -->
##### SQL:

<!-- अनुरोध SQL -->

```sql
SELECT HIGHLIGHT({},TO_STRING('कुछ टेक्स्ट हाइलाइट करने के लिए'), 'highlight') FROM books WHERE MATCH('@title one');
```

<!-- प्रतिक्रिया SQL -->
```sql
+----------------------------------------------------------------+
| highlight({},TO_STRING('कुछ टेक्स्ट हाइलाइट करने के लिए'), 'highlight') |
+----------------------------------------------------------------+
| कुछ टेक्स्ट को <b>हाइलाइट</b> करें                                  |
+----------------------------------------------------------------+
1 पंक्ति सेट में (0.00 सेकंड)
```

<!-- अंत -->

कई विकल्प केवल एकल स्ट्रिंग को परिणाम के रूप में उत्पन्न करते समय प्रासंगिक होते हैं (निविदाओं के एक एरे नहीं)। यह विशेष रूप से SQL `HIGHLIGHT()` फ़ंक्शन पर लागू होता है:

#### snippet_separator
निविदाओं के बीच डालने के लिए एक स्ट्रिंग। डिफ़ॉल्ट है ` ... `।
#### field_separator
फ़ील्ड के बीच डालने के लिए एक स्ट्रिंग। डिफ़ॉल्ट है `|`।


टेक्स्ट को हाइलाइट करने का एक और तरीका [CALL SNIPPETS](../Searching/Highlighting.md#CALL-SNIPPETS) कथन का उपयोग करना है। यह मुख्यतः `HIGHLIGHT()` कार्यक्षमता को डुप्लिकेट करता है लेकिन इसमें अंतर्निहित दस्तावेज़ भंडारण का उपयोग नहीं कर सकता। हालाँकि, यह फ़ाइलों से स्रोत टेक्स्ट लोड कर सकता है।


## HTTP के माध्यम से हाइलाइटिंग

<!-- JSON में हाइलाइट का उदाहरण -->

HTTP के माध्यम से JSON क्वेरी में पूर्ण-पाठ खोज परिणामों को हाइलाइट करने के लिए, फ़ील्ड सामग्री को दस्तावेज़ भंडारण में संग्रहीत किया जाना चाहिए (जो डिफ़ॉल्ट रूप से सक्षम है)। उदाहरण में, पूर्ण-पाठ क्षेत्रों `content` और `title` को दस्तावेज़ भंडारण से लाया जाता है और `query` क्लॉज में निर्दिष्ट क्वेरी के खिलाफ हाइलाइट किया जाता है।

हाइलाइट की गई निविदाएँ `hits` सरणी के `highlight` गुण में लौटाई जाती हैं।

<!-- परिचय -->
##### JSON:

<!-- अनुरोध JSON -->

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

<!-- प्रतिक्रिया JSON -->

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
          "title": "किताबें एक",
          "content": "वे बैंडर का पीछा कर रहे थे। रोबोट्स एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस होने वाला खतरा थी। बैंडर ने तीनों को कमरे में ले गया। एक रोबोट भी पीछा किया। बैंडर ने अन्य रोबोट्स को दूर किया और खुद प्रवेश किया। दरवाजा उसके पीछे बंद हो गया। "
        },
        "highlight": {
          "content": [
            "वे बैंडर का पीछा कर रहे थे। <b>रोबोट्स</b> एक विनम्र दूरी पर रहे, ",
            " तीनों को कमरे में। <b>एक</b> <b>रोबोट</b> भी पीछा किया। बैंडर",
            " ने अन्य <b>रोबोट</b> को दूर किया और खुद प्रवेश किया। द"
          ]
        }
      }
    ]
  }
}
```
<!-- परिचय -->
##### PHP:

<!-- अनुरोध PHP -->

```php
$index->setName('books');
$results = $index->search('one|robots')->highlight(['content'])->get();
foreach($results as $doc)
{
    echo 'दस्तावेज़: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "हाइलाइट के लिए ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}

```

<!-- प्रतिक्रिया PHP -->
```php
दस्तावेज़: 1
title : किताबें एक
content : वे बैंडर का पीछा कर रहे थे। रोबोट्स एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस होने वाला खतरा थी। बैंडर ने तीनों को कमरे में ले गया। एक रोबोट भी पीछा किया। बैंडर ने अन्य रोबोट्स को दूर किया और खुद प्रवेश किया। दरवाजा उसके पीछे बंद हो गया।
हाइलाइट के लिए सामग्री:
- वे बैंडर का पीछा कर रहे थे। <b>रोबोट्स</b> एक विनम्र दूरी पर रहे,
-  तीनों को कमरे में। <b>एक</b> <b>रोबोट</b> भी पीछा किया। बैंडर
-  ने अन्य <b>रोबोट</b> को दूर किया और खुद प्रवेश किया। द

```
<!-- अनुरोध Python -->
``` python
res = searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content"]}}))
```
<!-- प्रतिक्रिया Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 2788,
                    u'_source': {u'content': u'वे बैंडर का पीछा कर रहे थे। रोबोट्स एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस होने वाला खतरा थी। बैंडर ने तीनों को कमरे में ले गया। एक रोबोट भी पीछा किया। बैंडर ने अन्य रोबोट्स को दूर किया और खुद प्रवेश किया। दरवाजा उसके पीछे बंद हो गया। ',
                                 u'title': u'किताबें एक'},
                    u'highlight': {u'content': [u'वे बैंडर का पीछा कर रहे थे। <b>रोबोट्स</b> एक विनम्र दूरी पर रहे, ',
                                                u' तीनों को कमरे में। <b>एक</b> <b>रोबोट</b> भी पीछा किया। बैंडर',
                                                u' ने अन्य <b>रोबोट्स</b> को दूर किया और खुद प्रवेश किया। द']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- अनुरोध Javascript -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"*":"one|robots"}},"highlight":{"fields":["content"]}});
```
<!-- प्रतिक्रिया Javascript -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"पुस्तकें एक","content":"वे बंडर का पीछा करते रहे। रोबोट एक सम्माननीय दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस की जाने वाली खतरा थी। बंडर ने तीनों को कमरे में दाखिल कराया। एक और रोबोट भी पीछा किया। बंडर ने अन्य रोबोटों को इशारा किया और खुद अंदर गया। दरवाजा इसके पीछे बंद हो गया। "},"highlight":{"content":["वे बंडर का पीछा करते रहे। <b>रोबोट</b> एक सम्माननीय दूरी पर रहे, "," कमरे में दाखिल कराया। <b>एक</b> और <b>रोबोट</b> भी पीछा किया। बंडर"," ने अन्य <b>रोबोट</b] को इशारा किया और खुद अंदर गया। दरवाजा"]}}]}}
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
        hits: [{_id=1, _score=2788, _source={title=पुस्तकें एक, content=वे बंडर का पीछा करते रहे। रोबोट एक सम्माननीय दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस की जाने वाली खतरा थी। बंडर ने तीनों को कमरे में दाखिल कराया। एक और रोबोट भी पीछा किया। बंडर ने अन्य रोबोटों को इशारा किया और खुद अंदर गया। दरवाजा इसके पीछे बंद हो गया। }, highlight={title=[पुस्तकें <b>एक</b>], content=[वे बंडर का पीछा करते रहे। <b>रोबोट</b> एक सम्माननीय दूरी पर रहे, ,  कमरे में दाखिल कराया। <b>एक</b> और <b>रोबोट</b> भी पीछा किया। बंडर,  ने अन्य <b>रोबोट</b> को इशारा किया और खुद अंदर गया। दरवाजा]} 
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
        hits: [{_id=1, _score=2788, _source={title=पुस्तकें एक, content=वे बंडर का पीछा करते रहे। रोबोट एक सम्माननीय दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस की जाने वाली खतरा थी। बंडर ने तीनों को कमरे में दाखिल कराया। एक और रोबोट भी पीछा किया। बंडर ने अन्य रोबोटों को इशारा किया और खुद अंदर गया। दरवाजा इसके पीछे बंद हो गया। }, highlight={title=[पुस्तकें <b>एक</b>], content=[वे बंडर का पीछा करते रहे। <b>रोबोट</b> एक सम्माननीय दूरी पर रहे, ,  कमरे में दाखिल कराया। <b>एक</b> और <b>रोबोट</b> भी पीछा किया। बंडर,  ने अन्य <b>रोबोट</b> को इशारा किया और खुद अंदर गया। दरवाजा]} 
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
          "title": "पुस्तकें एक",
          "content": "वे बंडर का पीछा करते रहे। रोबोट एक सम्माननीय दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस की जाने वाली खतरा थी। बंडर ने तीनों को कमरे में दाखिल कराया। एक और रोबोट भी पीछा किया। बंडर ने अन्य रोबोटों को इशारा किया और खुद अंदर गया। दरवाजा इसके पीछे बंद हो गया। "
        },
        "highlight": {
          "title": [
            "पुस्तकें <b>एक</b>"
          ],
          "content": [
            "वे बंडर का पीछा करते रहे। <b>रोबोट</b> एक सम्माननीय दूरी पर रहे, ",
            " कमरे में दाखिल कराया। <b>एक</b> और <b>रोबोट</b> भी पीछे किया। बंडर",
            " ने अन्य <b>रोबोट</b> को इशारा किया और खुद अंदर गया। दरवाजा"
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
content : उन्होंने बेंडर का पीछा किया। रोबोट एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस किया गया खतरा था। बेंडर ने सभी तीनों को कमरे में भेजा। एक रोबोट भी उसके पीछे था। बेंडर ने अन्य रोबोटों को दूर किया और स्वयं अंदर गया। दरवाजा इसके पीछे बंद हो गया।
Highlight for title:
- Books <b>one</b>
Highlight for content:
- उन्होंने बेंडर का पीछा किया। <b>रोबोट</b> एक विनम्र दूरी पर रहे,
-  तीनों को कमरे में। <b>एक</b> <b>रोबोट</b> भी उसके पीछे था। बेंडर
-  ने अन्य <b>रोबोट</b> को दूर किया और स्वयं अंदर गया। 
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
                    u'_source': {u'content': u'उन्होंने बेंडर का पीछा किया। रोबोट एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस किया गया खतरा था। बेंडर ने सभी तीनों को कमरे में भेजा। एक रोबोट भी उसके पीछे था। बेंडर ने अन्य रोबोटों को दूर किया और स्वयं अंदर गया। दरवाजा इसके पीछे बंद हो गया। ',
                                 u'title': u'Books one'},
                    u'highlight': {u'content': [u'उन्होंने बेंडर का पीछा किया। द<b>रोबोट</b> एक विनम्र दूरी पर रहे, ',
                                                u' तीनों को कमरे में। <b>एक</b> <b>रोबोट</b> भी उसके पीछे था। बेंडर',
                                                u' ने अन्य <b>रोबोट</b> को दूर किया और स्वयं अंदर गया।'],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"Books one","content":"उन्होंने बेंडर का पीछा किया। रोबोट एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस किया गया खतरा था। बेंडर ने सभी तीनों को कमरे में भेजा। एक रोबोट भी उसके पीछे था। बेंडर ने अन्य रोबोटों को दूर किया और स्वयं अंदर गया। दरवाजा इसके पीछे बंद हो गया। "},"highlight":{"title":["Books <b>one</b>"],"content":["उन्होंने बेंडर का पीछा किया। द<b>रोबोट</b> एक विनम्र दूरी पर रहे, "," तीनों को कमरे में। <b>एक</b> <b>रोबोट</b> भी उसके पीछे था। बेंडर"," ने अन्य <b>रोबोट</b> को दूर किया और स्वयं अंदर गया।"]}}]}}

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
        hits: [{_id=1, _score=2788, _source={title=Books one, content=उन्होंने बेंडर का पीछा किया। रोबोट एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस किया गया खतरा था। बेंडर ने सभी तीनों को कमरे में भेजा। एक रोबोट भी उसके पीछे था। बेंडर ने अन्य रोबोटों को दूर किया और स्वयं अंदर गया। दरवाजा इसके पीछे बंद हो गया। }, highlight={title=[Books <b>one</b>], content=[उन्होंने बेंडर का पीछा किया। द<b>रोबोट</b> एक विनम्र दूरी पर रहे, ,  तीनों को कमरे में। <b>एक</b> <b>रोबोट</b> भी उसके पीछे था। बेंडर,  ने अन्य <b>रोबोट</b> को दूर किया और स्वयं अंदर गया।]}]}]
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
        hits: [{_id=1, _score=2788, _source={title=Books one, content=उन्होंने बेंडर का पीछा किया। रोबोट एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस किया गया खतरा था। बेंडर ने सभी तीनों को कमरे में भेजा। एक रोबोट भी उसके पीछे था। बेंडर ने अन्य रोबोटों को दूर किया और स्वयं अंदर गया। दरवाजा इसके पीछे बंद हो गया। }, highlight={title=[Books <b>one</b>], content=[उन्होंने बेंडर का पीछा किया। द<b>रोबोट</b> एक विनम्र दूरी पर रहे, ,  तीनों को कमरे में। <b>एक</b> <b>रोबोट</b> भी उसके पीछे था। बेंडर,  ने अन्य <b>रोबोट</b> को दूर किया और स्वयं अंदर गया।]}]}]
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

In addition to common highlighting options, several synonyms are available for JSON queries via HTTP:

#### fields
The `fields` object contains attribute names with options. It can also be an array of field names (without any options).

Note that by default, highlighting attempts to highlight the results following the full-text query. In a general case, when you don't specify fields to highlight, the highlight is based on your full-text query. However, if you specify fields to highlight, it highlights only if the full-text query matches the selected fields.

#### encoder
The `encoder` can be set to `default` or `html`. When set to `html`, it retains HTML markup when highlighting. This works similarly to the `html_strip_mode=retain` option.

<!-- example highlight_query -->
#### highlight_query
The `highlight_query` option allows you to highlight against a query other than your search query. The syntax is the same as in the main `query`.

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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"किताबें एक","content":"उन्हें बैंडर का अनुसरण किया। रोबोट एक कृत्रिम दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस की गई खतराहट थी। बैंडर ने तीनों को कमरे में अंदर बुलाया। एक रोबोट ने भी अनुसरण किया। बैंडर ने अन्य रोबोटों को दूर इशारा किया और खुद अंदर गया। दरवाजा उसके पीछे बंद हो गया। "},"highlight":{"content":["उन्हें बैंडर का अनुसरण किया। before_रोबोट_after एक कृत्रिम दूरी पर रहे, "," तीनों को कमरे में अंदर। before_एक_after का before_रोबोट_after ने भी अनुसरण किया। बैंडर"," ने अन्य before_रोबोट_after को दूर इशारा किया और खुद अंदर गया। The"],"title":["किताबें before_एक_after"]}}]}}
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
`no_match_size` functions similarly to the `allow_empty` option. If set to 0, it acts as `allow_empty=1`, allowing an empty string to be returned as a highlighting result when a snippet could not be generated. Otherwise, the beginning of the field will be returned. This is optional, with a default value of 1.

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
title : किताबें एक
content : उन्होंने बैंडर का अनुसरण किया। रोबोट एक कृत्रिम दूरी पर रहे, लेकिन उनकी उपस्थिति एक लगातार महसूस की गई खतराहट थी। बैंडर ने तीनों को कमरे में अंदर बुलाया। एक रोबोट ने भी अनुसरण किया। बैंडर ने अन्य रोबोटों को दूर इशारा किया और खुद अंदर गया। दरवाजा उसके पीछे बंद हो गया।
Highlight for content:
- उन्होंने बैंडर का अनुसरण किया। The <b>रोबोट</b> एक कृत्रिम दूरी पर रहे,
-  तीनों को कमरे में अंदर। <b>एक</b> का <b>रोबोट</b> ने भी अनुसरण किया। बैंडर
-  ने अन्य <b>रोबोट</b> को दूर इशारा किया और खुद अंदर गया। The
Highlight for title:
- किताबें <b>एक</b>
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
                    u'_source': {u'content': u'वे बैंडर के पीछे चले। रोबोट एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति लगातार महसूस होने वाला खतरा थी। बैंडर ने तीनों को कमरे में बुलाया। एक रोबोट भी पीछे आया। बैंडर ने अन्य रोबोटों को दूर करने का इशारा किया और खुद अंदर गया। दरवाजा उसके पीछे बंद हो गया। ',
                                 u'title': u'पुस्तकें एक'},
                    u'highlight': {u'content': [u'वे बैंडर के पीछे चले। <b>रोबोट</b> एक विनम्र दूरी पर रहे, ',
                                                u' तीनों को कमरे में। <b>एक</b> <b>रोबोट</b> भी पीछे आया। बैंडर',
                                                u' ने अन्य <b>रोबोट</b>ों को दूर करने का इशारा किया और खुद अंदर गया। वह'],
                                   u'title': [u'पुस्तकें <b>एक</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"पुस्तकें एक","content":"वे बैंडर के पीछे चले। रोबोट एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति लगातार महसूस होने वाला खतरा थी। बैंडर ने तीनों को कमरे में बुलाया। एक रोबोट भी पीछे आया। बैंडर ने अन्य रोबोटों को दूर करने का इशारा किया और खुद अंदर गया। दरवाजा उसके पीछे बंद हो गया। "},"highlight":{"content":["वे बैंडर के पीछे चले। <b>रोबोट</b> एक विनम्र दूरी पर रहे, "," तीनों को कमरे में। <b>एक</b> <b>रोबोट</b> भी पीछे आया। बैंडर"," ने अन्य <b>रोबोट</b>ों को दूर करने का इशारा किया और खुद अंदर गया। वह"],"title":["पुस्तकें <b>एक</b>"]}}]}}

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
title : पुस्तकें एक
content : वे बैंडर के पीछे चले। रोबोट एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति लगातार महसूस होने वाला खतरा थी। बैंडर ने तीनों को कमरे में बुलाया। एक रोबोट भी पीछे आया। बैंडर ने अन्य रोबोटों को दूर करने का इशारा किया और खुद अंदर गया। दरवाजा उसके पीछे बंद हो गया।
Highlight for content:
-  तीनों को कमरे में। <b>एक</b> <b>रोबोट</b> भी पीछे आया। बैंडर
-  अन्य <b>रोबोटों</b> की ओर इशारा किया और स्वयं अंदर गया। यह
- वे बैंडर का पालन करते रहे। <b>रोबोट</b> शिष्ट दूरी पर बने रहे,
शीर्षक के लिए हाइलाइट:
- पुस्तकें <b>एक</b>
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
                    u'_source': {u'content': u'वे बैंडर का पालन करते रहे। रोबोट शिष्ट दूरी पर बने रहे, लेकिन उनकी उपस्थिति निरंतर एक खतरे के रूप में महसूस होती रही। बैंडर ने सभी तीन को कमरे में usher किया। एक रोबोट भी उसका पालन करता रहा। बैंडर ने अन्य रोबोटों की ओर इशारा किया और स्वयं अंदर गया। दरवाजा इसके पीछे बंद हो गया। ',
                                 u'title': u'पुस्तकें एक'},
                    u'highlight': {u'content': [u' तीन को कमरे में। <b>एक</b> रोबोट भी उसका पालन करता रहा। बैंडर',
                                                u' अन्य <b>रोबोटों</b> की ओर इशारा किया और स्वयं अंदर गया। यह',
                                                u'वे बैंडर का पालन करते रहे। <b>रोबोट</b> शिष्ट दूरी पर बने रहे, '],
                                   u'title': [u'पुस्तकें <b>एक</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"पुस्तकें एक","content":"वे बैंडर का पालन करते रहे। रोबोट शिष्ट दूरी पर बने रहे, लेकिन उनकी उपस्थिति निरंतर एक खतरे के रूप में महसूस होती रही। बैंडर ने सभी तीन को कमरे में usher किया। एक रोबोट भी उसका पालन करता रहा। बैंडर ने अन्य रोबोटों की ओर इशारा किया और स्वयं अंदर गया। दरवाजा इसके पीछे बंद हो गया। "},"highlight":{"content":[" तीन को कमरे में। <b>एक</b> रोबोट भी उसका पालन करता रहा। बैंडर"," अन्य <b>रोबोटों</b> की ओर इशारा किया और स्वयं अंदर गया। यह","वे बैंडर का पालन करते रहे। <b>रोबोट</b> शिष्ट दूरी पर बने रहे, "],"title":["पुस्तकें <b>एक</b>"]}}]}}


```
<!-- intro -->
##### जावा:

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
`fragment_size` अधिकतम स्निपेट आकार को प्रतीकों में सेट करता है। यह वैश्विक या प्रति-क्षेत्र हो सकता है। प्रति-क्षेत्र विकल्प वैश्विक विकल्पों को अधिलेखित करता है। यह वैकल्पिक है, जिसमें डिफ़ॉल्ट मान 256 है। यह `limit` विकल्प के समान काम करता है।

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
दस्तावेज़: 1
शीर्षक : किताबें एक
सामग्री : उन्होंने बैंडे का पालन किया। रोबोट एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति लगातार महसूस होने वाला खतरा थी। बैंडे ने सभी तीनों को कमरे में भेजा। एक रोबोट भी उनके साथ आया। बैंडे ने अन्य रोबोटों को इशारा किया और खुद अंदर चले गए। दरवाजा उनके पीछे बंद हो गया।
सामग्री के लिए हाइलाइट:
-  कमरा। <b>एक</b> <b>रोबोट</b> भी उनके साथ आया
- बैंडे ने अन्य <b>रोबोटों</b> को दूर किया और प्रवेश किया
शीर्षक के लिए हाइलाइट:
- किताबें <b>एक</b>
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
                    u'_source': {u'content': u'उन्होंने बैंडे का पालन किया। रोबोट एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति लगातार महसूस होने वाला खतरा थी। बैंडे ने सभी तीनों को कमरे में भेजा। एक रोबोट भी उनके साथ आया। बैंडे ने अन्य रोबोटों को इशारा किया और खुद अंदर चले गए। दरवाजा उनके पीछे बंद हो गया। ',
                                 u'title': u'किताबें एक'},
                    u'highlight': {u'content': [u' कमरा। <b>एक</b> <b>रोबोट</b> भी उनके साथ आया',
                                                u'बैंडे ने अन्य <b>रोबोटों</b> को दूर किया और प्रवेश किया '],
                                   u'title': [u'किताबें <b>एक</b>']}}],
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
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":2788,"_source":{"title":"किताबें एक","content":"उन्होंने बैंडे का पालन किया। रोबोट एक विनम्र दूरी पर रहे, लेकिन उनकी उपस्थिति लगातार महसूस होने वाला खतरा थी। बैंडे ने सभी तीनों को कमरे में भेजा। एक रोबोट भी उनके साथ आया। बैंडे ने अन्य रोबोटों को इशारा किया और खुद अंदर चले गए। दरवाजा उनके पीछे बंद हो गया। "},"highlight":{"content":[" कमरा। <b>एक</b> <b>रोबोट</b> भी उनके साथ आया","बैंडे ने अन्य <b>रोबोटों</b> को दूर किया और प्रवेश किया "],"title":["किताबें <b>एक</b>"]}}]}}
```
<!-- intro -->
##### जावा:

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

<!-- अंत -->

<!-- उदाहरण number_of_fragments -->
#### number_of_fragments
`number_of_fragments` परिणाम में अधिकतम स्निप्पेट्स की संख्या को सीमित करता है। जैसे `fragment_size`, यह वैश्विक या प्रति-क्षेत्र हो सकता है। यह वैकल्पिक है, जिसका डिफ़ॉल्ट मान 0 (कोई सीमा नहीं) है। यह `limit_snippets` विकल्प के समान काम करता है।

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
    echo 'दस्तावेज़: '.$doc->getId()."\n";
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
					"<b>पाठ</b>"
				]
			}
		]}
	}
}
```

<!-- अंत -->

<!-- उदाहरण हाइलाइट JSON वैश्विक सीमाएँ -->

#### सीमाएँ_प्रति_क्षेत्र
वैश्विक सीमाएँ `limits_per_field=0` निर्दिष्ट करके भी लागू की जा सकती हैं। इस विकल्प को सेट करना का मतलब है कि सभी संयुक्त हाइलाइटिंग परिणाम निर्दिष्ट सीमाओं के भीतर होना चाहिए। नुकसान यह है कि यदि हाइलाइटिंग इंजन यह तय करता है कि वे अधिक प्रासंगिक हैं, तो आपको एक क्षेत्र में कई स्निप्पेट हाइलाइट किए जा सकते हैं और दूसरी में कोई नहीं।

<!-- परिचय -->
##### JSON:
<!-- अनुरोध JSON -->

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
<!-- अनुरोध PHP -->

```php
$index->setName('books');
$bool = new \Manticoresearch\Query\BoolQuery();
$bool->must(new \Manticoresearch\Query\Match(['query' => 'and first'], 'content'));

$results = $index->search($bool)->highlight(['content'=>['limit'=>50]],['limits_per_field'=>false])->get();
foreach($results as $doc)
{
    echo 'दस्तावेज़: '.$doc->getId()."\n";
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.' : '.$value."\n";
    }
    foreach($doc->getHighlight() as $field=>$snippets)
    {
        echo "हाइलाइट के लिए ".$field.":\n";
        foreach($snippets as $snippet)
        {
            echo "- ".$snippet."\n";
        }
    }
}
```
<!-- प्रतिक्रिया PHP -->
```php
दस्तावेज़: 1
शीर्षक : किताबें एक
सामग्री : उन्होंने बैंडर का अनुसरण किया। रोबोट शिष्टता की दूरी पर बने रहे, लेकिन उनका होना लगातार महसूस किया गया खतरा था। बैंडर ने तीनों को कमरे में भेजा। एक रोबोट भी अनुसरण किया। बैंडर ने अन्य रोबोटों को दूर किया और स्वयं में प्रवेश किया। पीछे दरवाजा बंद हो गया।
सामग्री के लिए हाइलाइट:
-  अन्य रोबोटों को दूर किया <b>और</b> स्वयं में प्रवेश किया। दरवाजा बंद
```
<!-- अनुरोध पाइथन -->
``` python
res =searchApi.search({"table":"books","query":{"match":{"content":"and first"}},"highlight":{"fields":{"content":{"limit":50}},"limits_per_field":False}})
```
<!-- प्रतिक्रिया पाइथन -->
``` python
{'aggregations': None,
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1597,
                    u'_source': {u'content': u'उन्होंने बैंडर का अनुसरण किया। रोबोट शिष्टता की दूरी पर बने रहे, लेकिन उनका होना लगातार महसूस किया गया खतरा था। बैंडर ने तीनों को कमरे में भेजा। एक रोबोट भी अनुसरण किया। बैंडर ने अन्य रोबोटों को दूर किया और स्वयं में प्रवेश किया। पीछे दरवाजा बंद हो गया। ',
                                 u'title': u'किताबें एक'},
                    u'highlight': {u'content': [u' अन्य रोबोटों को दूर किया <b>और</b> स्वयं में प्रवेश किया। दरवाजा बंद']}}],
          'max_score': None,
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- अनुरोध जावास्क्रिप्ट -->
``` javascript
res =  await searchApi.search({"table":"books","query":{"match":{"content":"and first"}},"highlight":{"fields":{"content":{"limit":50}},"limits_per_field":false}});
```
<!-- प्रतिक्रिया जावास्क्रिप्ट -->
``` javascript
{"took":0,"timed_out":false,"hits":{"total":1,"hits":[{"_id": 1,"_score":1597,"_source":{"title":"किताबें एक","content":"उन्होंने बैंडर का अनुसरण किया। रोबोट शिष्टता की दूरी पर बने रहे, लेकिन उनका होना लगातार महसूस किया गया खतरा था। बैंडर ने तीनों को कमरे में भेजा। एक रोबोट भी अनुसरण किया। बैंडर ने अन्य रोबोटों को दूर किया और स्वयं में प्रवेश किया। पीछे दरवाजा बंद हो गया। "},"highlight":{"content":[" अन्य रोबोटों को दूर किया <b>और</b> स्वयं में प्रवेश किया। दरवाजा बंद"]}}]}}

```
<!-- परिचय -->
##### जावा:

<!-- अनुरोध जावा -->

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

<!-- परिचय -->
##### C#:

<!-- अनुरोध C# -->

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

<!-- अनुरोध टाइपस्क्रिप्ट -->
``` typescript
res = await searchApi.search({
  index: 'test',
  query: {
    match: {
      *: 'पाठ 1'
    }
  },
  highlight: { limits_per_field: 0 }
});
```

<!-- अनुरोध गो -->
``` go
matchClause := map[string]interface{} {"*": "पाठ 1"};
query := map[string]interface{} {"match": matchClause};
searchRequest.SetQuery(query);
highlight := manticoreclient.NewHighlight()
highlight.SetLimitsPerField(0)
searchRequest.SetHighlight(highlight)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- अंत -->

# कॉल स्निप्पेट्स

<!-- उदाहरण कॉल स्निप्पेट्स -->

`CALL SNIPPETS` कथन निर्दिष्ट तालिका सेटिंग्स का उपयोग करके प्रदान किए गए डेटा और प्रश्न से स्निप्पेट बनाता है। यह अंतर्निहित दस्तावेज़ भंडारण तक पहुँच नहीं सकता है, यही कारण है कि [HIGHLIGHT() function](../Searching/Highlighting.md) का उपयोग करने की सिफारिश की जाती है।

वाक्यविन्यास है:

```sql
CALL SNIPPETS(data, table, query[, opt_value AS opt_name[, ...]])
```

#### data
`data` उस स्रोत के रूप में कार्य करता है जिससे एक स्निपेट निकाला जाता है। यह या तो एकल स्ट्रिंग हो सकता है या कैली ब्रेसिज़ में बंद स्ट्रिंग की एक सूची हो सकती है।
#### table
`table` उस टेबल के नाम को संदर्भित करता है जो स्निपेट उत्पादन के लिए टेक्स्ट प्रोसेसिंग सेटिंग्स प्रदान करता है।
#### query
`query` वह पूर्ण-पाठ क्वेरी है जिसका उपयोग स्निपेट बनाने के लिए किया जाता है।
#### opt_value और opt_name
`opt_value` और `opt_name` [स्निपेट उत्पादन विकल्पों](../Searching/Highlighting.md) का प्रतिनिधित्व करते हैं।

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

अधिकांश विकल्प [HIGHLIGHT() function](../Searching/Highlighting.md) के समान हैं। हालांकि, कई विकल्प हैं जो केवल `CALL SNIPPETS` के साथ उपयोग किए जा सकते हैं।

<!-- example CALL SNIPPETS load files -->
निम्नलिखित विकल्पों का उपयोग अलग फ़ाइलों में संग्रहीत पाठ को हाईलाइट करने के लिए किया जा सकता है:

#### load_files
यह विकल्प, जब सक्षम हो, पहले तर्क को स्निपेट निकालने के लिए डेटा के बजाय फ़ाइल नामों के रूप में मानता है। सर्वर साइड पर निर्दिष्ट फ़ाइलें डेटा के लिए लोड की जाएंगी। इस ध्वज को सक्षम करने पर प्रत्येक अनुरोध के लिए [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) कार्यकर्ता थ्रेड्स का उपयोग कार्य को समानांतर करने के लिए किया जाएगा। डिफ़ॉल्ट 0 है (कोई सीमा नहीं)। दूरस्थ एजेंटों के बीच स्निपेट उत्पादन का वितरण करने के लिए, एक वितरित तालिका में स्निपेट संचालन करें जिसमें केवल एक(!) स्थानीय एजेंट और कई दूरस्थ एजेंट हों। [snippets_file_prefix](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#snippets_file_prefix) विकल्प अंतिम फ़ाइल नाम उत्पन्न करने के लिए उपयोग किया जाता है। उदाहरण के लिए, जब searchd को `snippets_file_prefix = /var/data_` के साथ कॉन्फ़िगर किया जाता है और `text.txt` को फ़ाइल नाम के रूप में प्रदान किया जाता है, स्निपेट `/var/data_text.txt` के सामग्री से उत्पन्न किए जाएंगे।

#### load_files_scattered
यह विकल्प केवल दूरस्थ एजेंटों के साथ वितरित स्निपेट उत्पादन के साथ काम करता है। स्निपेट उत्पादन के लिए स्रोत फ़ाइलें विभिन्न एजेंटों के बीच वितरित की जा सकती हैं, और मुख्य सर्वर सभी गैर-त्रुटिपूर्ण परिणामों को विलीन करेगा। उदाहरण के लिए, यदि वितरित तालिका का एक एजेंट `file1.txt` है, दूसरा एजेंट `file2.txt` है, और आप इन दोनों फ़ाइलों के साथ `CALL SNIPPETS` का उपयोग करते हैं, तो searchd एजेंट के परिणामों को विलीन करेगा, ताकि आपको `file1.txt` और `file2.txt` दोनों से परिणाम मिलें। डिफ़ॉल्ट 0 है।

यदि `load_files` विकल्प भी सक्षम है, तो अनुरोध एक त्रुटि लौटाएगा यदि किसी भी फ़ाइल कहीं भी उपलब्ध नहीं है। अन्यथा (यदि `load_files` सक्षम नहीं है), यह सभी अनुपस्थित फ़ाइलों के लिए खाली स्ट्रिंग लौटाएगा। Searchd इस ध्वज को एजेंटों को पास नहीं करता है, इसलिए एजेंट यदि फ़ाइल मौजूद नहीं है तो एक महत्वपूर्ण त्रुटि उत्पन्न नहीं करते हैं। यदि आप सुनिश्चित करना चाहते हैं कि सभी स्रोत फ़ाइलें लोड की गई हैं, तो दोनों `load_files_scattered` और `load_files` को 1 पर सेट करें। यदि कुछ एजेंट पर कुछ स्रोत फ़ाइलों की अनुपस्थिति महत्वपूर्ण नहीं है, तो केवल `load_files_scattered` को 1 पर सेट करें।

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

