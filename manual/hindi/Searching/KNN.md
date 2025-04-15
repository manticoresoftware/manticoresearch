# K-nearest neighbor vector search
Manticore Search आपके मशीन लर्निंग मॉडल द्वारा उत्पन्न एम्बेडिंग को प्रत्येक दस्तावेज़ में जोड़ने और फिर उनके ऊपर निकटतम पड़ोसी खोज करने की क्षमता का समर्थन करता है। यह आपको समानता खोज, सिफारिशें, अर्थपूर्ण खोज, और एनएलपी एल्गोरिदम के आधार पर प्रासंगिकता रैंकिंग जैसी सुविधाएँ बनाने देता है, जिनमें छवि, वीडियो, और ध्वनि खोजें शामिल हैं।
## एम्बेडिंग क्या है?
एक एम्बेडिंग डेटा का प्रतिनिधित्व करने की एक विधि है—जैसे कि पाठ, छवियाँ, या ध्वनि—को उच्च-आयामी स्पेस में वेक्टर के रूप में। ये वेक्टर इस तरह से तैयार किए जाते हैं कि उनके बीच की दूरी उस डेटा की समानता को दर्शाती है जिसे वे दर्शाते हैं। इस प्रक्रिया में आमतौर पर पाठ के लिए शब्द एम्बेडिंग (जैसे, Word2Vec, BERT) या छवियों के लिए न्यूरल नेटवर्क जैसे एल्गोरिदम का उपयोग किया जाता है। वेक्टर स्पेस की उच्च-आयामी प्रकृति, प्रत्येक वेक्टर में कई घटकों के साथ, वस्तुओं के बीच जटिल और बारीक संबंधों का प्रतिनिधित्व करने की अनुमति देती है। उनकी समानता को इन वेक्टरों के बीच की दूरी के द्वारा मापा जाता है, जिसे अक्सर यूक्लिडियन दूरी या कोसाइन समानता जैसे तरीकों से मापा जाता है।
Manticore Search HNSW लाइब्रेरी का उपयोग करके k-nearest neighbor (KNN) वेक्टर खोजों को सक्षम करता है। यह कार्यक्षमता [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) का एक हिस्सा है।
<!-- example KNN -->
### KNN खोज के लिए तालिका को कॉन्फ़िगर करना
KNN खोजें चलाने के लिए, आपको पहले अपनी तालिका को कॉन्फ़िगर करना होगा। इसमें कम से कम एक float_vector विशेषता होनी चाहिए, जो एक डेटा वेक्टर के रूप में काम करती है। आपको निम्नलिखित गुण निर्दिष्ट करने की आवश्यकता है:
* `knn_type`: एक अनिवार्य सेटिंग; वर्तमान में, केवल `hnsw` समर्थित है।
* `knn_dims`: एक अनिवार्य सेटिंग जो अनुक्रमित वेक्टर के आयामों का निर्दिष्ट करती है।
* `hnsw_similarity`: एक अनिवार्य सेटिंग जो HNSW अनुक्रमणिका द्वारा उपयोग की जाने वाली दूरी फ़ंक्शन को निर्दिष्ट करती है। स्वीकार्य मान हैं:
  - `L2` - वर्ग L2
  - `IP` - आंतरिक उत्पाद
  - `COSINE` - कोसाइन समानता
* `hnsw_m`: एक वैकल्पिक सेटिंग जो ग्राफ में अधिकतम आउटगोइंग कनेक्शनों की संख्या को परिभाषित करती है। डिफ़ॉल्ट 16 है।
* `hnsw_ef_construction`: एक वैकल्पिक सेटिंग जो निर्माण समय/सटीकता व्यापार-ऑफ को परिभाषित करती है।
<!-- intro -->
##### SQL:
<!-- request SQL -->
```sql
create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' );
```
<!-- response -->
```sql
Query OK, 0 rows affected (0.01 sec)
```
<!-- end -->
<!-- example knn_insert -->
### वेक्टर डेटा डालना
तालिका बनाने के बाद, आपको अपने वेक्टर डेटा को डालने की आवश्यकता है, यह सुनिश्चित करते हुए कि यह उन आयामों से मेल खाता है जो आपने तालिका बनाने के समय निर्दिष्ट किए थे।
<!-- intro -->
##### SQL:
<!-- request SQL -->
```sql
insert into test values ( 1, 'yellow bag', (0.653448,0.192478,0.017971,0.339821) ), ( 2, 'white bag', (-0.148894,0.748278,0.091892,-0.095406) );
```
<!-- response SQL -->
```sql
Query OK, 2 rows affected (0.00 sec)
```
<!-- intro -->
##### JSON:
<!-- request JSON -->
```json
POST /insert
{
	"table":"test_vec",
	"id":1,
	"doc": 	{ "title" : "yellow bag", "image_vector" : [0.653448,0.192478,0.017971,0.339821] }
}
POST /insert
{
	"table":"test_vec",
	"id":2,
	"doc": 	{ "title" : "white bag", "image_vector" : [-0.148894,0.748278,0.091892,-0.095406] }
}
```
<!-- response JSON -->
```json
{
	"table":"test",
	"_id":1,
	"created":true,
	"result":"created",
	"status":201
}
{
	"table":"test",
	"_id":2,
	"created":true,
	"result":"created",
	"status":201
}
```
<!-- end -->
<!-- example knn_search -->
### KNN वेक्टर खोज
अब, आप SQL या JSON प्रारूप में `knn` क्लॉज का उपयोग करके KNN खोज कर सकते हैं। दोनों इंटरफेस समान आवश्यकताओं को समर्थन करते हैं, इस बात को सुनिश्चित करते हुए कि आप जिस प्रारूप को चुनते हैं, उसमें एक सुसंगत अनुभव है:
- SQL: `select ... from <table name> where knn ( <field>, <k>, <query vector> [,<ef>] )`
- JSON:
  ```
  POST /search
  {
      "table": "<table name>",
      "knn":
      {
          "field": "<field>",
          "query_vector": [<query vector>],
          "k": <k>,
          "ef": <ef>
      }
  }
  ```
पैरामीटर हैं:
* `field`: यह float वेक्टर विशेषता का नाम है जिसमें वेक्टर डेटा होता है।
* `k`: यह वे दस्तावेज़ों की संख्या दर्शाता है जिन्हें लौटाना है और यह हायेरार्चिकल नैविगेबल स्मॉल वर्ल्ड (HNSW) अनुक्रमिकाओं के लिए एक प्रमुख पैरामीटर है। यह निर्दिष्ट करता है कि एक एकल HNSW अनुक्रमणिका को कितने दस्तावेज़ लौटाने चाहिए। हालांकि, अंतिम परिणामों में शामिल दस्तावेज़ों की वास्तविक संख्या भिन्न हो सकती है। उदाहरण के लिए, यदि सिस्टम वास्तविक समय की तालिकाओं का सामना कर रहा है जो डिस्क के टुकड़ों में विभाजित हैं, तो प्रत्येक टुकड़ा `k` दस्तावेज़ लौटाने कर सकता है, जिससे कि कुल संख्या निर्दिष्ट `k` से अधिक हो सकती है (जैसा कि संचित गणना `num_chunks * k` होगी)। दूसरी ओर, यदि, `k` दस्तावेजों को मांगने के बाद, कुछ विशिष्ट विशेषताओं के आधार पर हटा दिए जाते हैं, तो अंतिम दस्तावेज़ संख्या `k` से कम हो सकती है। यह महत्वपूर्ण है कि पैरामीटर `k` ramchunks पर लागू नहीं होता है। ramchunks के संदर्भ में, पुनर्प्राप्ति प्रक्रिया अलग तरह से काम करती है, और इसलिए, दस्तावेज़ों की संख्या पर `k` पैरामीटर का प्रभाव लागू नहीं होता है।
* `query_vector`: यह खोज वेक्टर है।
* `ef`: खोज के दौरान उपयोग की जाने वाली गतिशील सूची का विकल्प आकार। उच्च `ef` अधिक सटीक लेकिन धीमी खोज की ओर ले जाता है।
दस्तावेज़ हमेशा खोज वेक्टर के साथ उनकी दूरी के अनुसार क्रमबद्ध होते हैं। कोई भी अतिरिक्त क्रमबद्धता मानदंड जिसे आप निर्दिष्ट करते हैं, इस प्राथमिक क्रम शर्त के बाद लागू किया जाएगा। दूरी को पुनर्प्राप्त करने के लिए, एक अंतर्निहित फ़ंक्शन है जिसे [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29) कहा जाता है।
<!-- intro -->
##### SQL:
<!-- request SQL -->
```sql
select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926), 2000 );
```
<!-- response SQL -->
```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    1 | 0.28146550 |
|    2 | 0.81527930 |
+------+------------+
2 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
	"table": "test",
	"knn":
	{
		"field": "image_vector",
		"query_vector": [0.286569,-0.031816,0.066684,0.032926],
		"k": 5,
		"ef": 2000
	}
}
```

<!-- response JSON -->

```json
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":2,
		"total_relation":"eq",
		"hits":
		[
			{
				"_id": 1,
				"_score":1,
				"_knn_dist":0.28146550,
				"_source":
				{
					"title":"पीला बैग",
					"image_vector":[0.653448,0.192478,0.017971,0.339821]
				}
			},
			{
				"_id": 2,
				"_score":1,
				"_knn_dist":0.81527930,
				"_source":
				{
					"title":"सफेद बैग",
					"image_vector":[-0.148894,0.748278,0.091892,-0.095406]
				}
			}
		]
	}
}
```

<!-- end -->

<!-- Example knn_similar_docs -->


### Find similar docs by id

> NOTE: Finding similar documents by id requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

Finding documents similar to a specific one based on its unique ID is a common task. For instance, when a user views a particular item, Manticore Search can efficiently identify and display a list of items that are most similar to it in the vector space. Here's how you can do it:

- SQL: `select ... from <table name> where knn ( <field>, <k>, <document id> )`
- JSON:
  ```
  POST /search
  {
      "table": "<table name>",
      "knn":
      {
          "field": "<field>",
          "doc_id": <document id>,
          "k": <k>
      }
  }
  ```

The parameters are:
* `field`: This is the name of the float vector attribute containing vector data.
* `k`: This represents the number of documents to return and is a key parameter for Hierarchical Navigable Small World (HNSW) indexes. It specifies the quantity of documents that a single HNSW index should return. However, the actual number of documents included in the final results may vary. For instance, if the system is dealing with real-time tables divided into disk chunks, each chunk could return `k` documents, leading to a total that exceeds the specified `k` (as the cumulative count would be `num_chunks * k`). On the other hand, the final document count might be less than `k` if, after requesting `k` documents, some are filtered out based on specific attributes. It's important to note that the parameter `k` does not apply to ramchunks. In the context of ramchunks, the retrieval process operates differently, and thus, the `k` parameter's effect on the number of documents returned is not applicable.
* `document id`: Document ID for KNN similarity search.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, 5, 1 );
```
<!-- response SQL -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    2 | 0.81527930 |
+------+------------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "test",
  "knn":
  {
    "field": "image_vector",
    "doc_id": 1,
    "k": 5
  }
}
```

<!-- response JSON -->

```json
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"total_relation":"eq",
		"hits":
		[
			{
				"_id": 2,
				"_score":1643,
				"_knn_dist":0.81527930,
				"_source":
				{
					"title":"सफेद बैग",
					"image_vector":[-0.148894,0.748278,0.091892,-0.095406]
				}
			}
		]
	}
}
```

<!-- end -->

<!-- Example knn_filtering -->

### Filtering KNN vector search results

Manticore also supports additional filtering of documents returned by the KNN search, either by full-text matching, attribute filters, or both.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926) ) and match('white') and id < 10;
```
<!-- response SQL -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    2 | 0.81527930 |
+------+------------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
	"table": "test",
	"knn":
	{
		"field": "image_vector",
		"query_vector": [0.286569,-0.031816,0.066684,0.032926],
		"k": 5,
		"filter":
		{
			"bool":
			{
				"must":
				[
					{ "match": {"_all":"सफेद"} },
			        { "range": { "id": { "lt": 10 } } }
				]
			}
		}
	}
}
```

<!-- response JSON -->

```json
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"total_relation":"eq",
		"hits":
		[
			{
				"_id": 2,
				"_score":1643,
				"_knn_dist":0.81527930,
				"_source":
				{
					"title":"सफेद बैग",
					"image_vector":[-0.148894,0.748278,0.091892,-0.095406]
				}
			}
		]
	}
}
```

<!-- end -->

<!-- proofread -->




































