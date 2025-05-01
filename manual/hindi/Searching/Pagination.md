# खोज परिणामों का पृष्ठन

<!-- उदाहरण सामान्य -->

मैन्टिकोर सर्च डिफ़ॉल्ट रूप से परिणाम सेट में शीर्ष 20 मिलान दस्तावेज़ लौटाता है।

#### SQL
SQL में, आप `LIMIT` क्लॉज़ का उपयोग करके परिणाम सेट के माध्यम से नेविगेट कर सकते हैं।

`LIMIT` एक संख्या को स्वीकार कर सकता है जो returned सेट के आकार के साथ जीरो ऑफसेट के रूप में है, या ऑफसेट और आकार मूल्यों के एक जोड़े को स्वीकार कर सकता है।

#### HTTP JSON
HTTP JSON का उपयोग करते समय, नोड्स `offset` और `limit` परिणाम सेट के ऑफसेट और लौटी गई सेट के आकार को नियंत्रित करते हैं। वैकल्पिक रूप से, आप इसके बजाय `size` और `from` का जोड़ा उपयोग कर सकते हैं।

<!-- परिचय -->

<!-- अनुरोध SQL -->

```sql
SELECT  ... FROM ...  [LIMIT [offset,] row_count]
SELECT  ... FROM ...  [LIMIT row_count][ OFFSET offset]
```


<!-- अनुरोध JSON -->

```json
{
  "table": "<table_name>",
  "query": ...
  ...  
  "limit": 20,
  "offset": 0
}
{
  "table": "<table_name>",
  "query": ...
  ...  
  "size": 20,
  "from": 0
}
```

<!-- अंत -->

<!-- उदाहरण maxMatches -->
### परिणाम सेट विंडो

डिफ़ॉल्ट रूप से, मैन्टिकोर सर्च परिणाम सेट में लौटाए जा सकने वाले 1000 सर्वोत्तम रैंक वाले दस्तावेजों की एक परिणाम सेट विंडो का उपयोग करता है। यदि परिणाम सेट इस मान से अधिक पृष्ठांकित है, तो क्वेरी त्रुटि में समाप्त हो जाएगी।

इस सीमा को क्वेरी विकल्प [max_matches](../Searching/Options.md#max_matches) के साथ समायोजित किया जा सकता है।

`max_matches` को बहुत उच्च मानों में बढ़ाना केवल तब करना चाहिए जब ऐसी स्थानों तक पहुंचने के लिए नेविगेशन आवश्यक हो। एक उच्च `max_matches` मान को अधिक मेमोरी की आवश्यकता होती है और यह क्वेरी प्रतिक्रिया समय को बढ़ा सकता है। गहरे परिणाम सेट के साथ काम करने का एक तरीका यह है कि `max_matches` को ऑफसेट और सीमा के योग के रूप में सेट किया जाए।

`max_matches` को 1000 से कम करना क्वेरी द्वारा उपयोग की जाने वाली मेमोरी को कम करने का लाभ देता है। यह क्वेरी समय को भी कम कर सकता है, लेकिन अधिकांश मामलों में, यह ध्यान देने योग्य लाभ नहीं हो सकता है।

<!-- परिचय -->


<!-- अनुरोध SQL -->

```sql
SELECT  ... FROM ...   OPTION max_matches=<value>
```


<!-- अनुरोध JSON -->


```json
{
  "table": "<table_name>",
  "query": ...
  ...
  "max_matches":<value>
  }
}

```

<!-- अंत -->

## स्क्रॉल सर्च विकल्प

स्क्रॉल सर्च विकल्प बड़े परिणाम सेट के माध्यम से पृष्ठांकित करने के लिए एक प्रभावशाली और विश्वसनीय तरीका प्रदान करता है। पारंपरिक ऑफसेट-आधारित पृष्ठन के विपरीत, स्क्रॉल सर्च गहरे पृष्ठन के लिए बेहतर प्रदर्शन प्रदान करता है और पृष्ठन को लागू करने का एक आसान तरीका प्रदान करता है।

#### SQL के माध्यम से स्क्रॉलिंग

<!-- उदाहरण scroll_sql_init -->
**क्रमबद्ध मानदंड के साथ प्रारंभिक क्वेरी**

अपनी इच्छित क्रमबद्धता के मानदंड के साथ प्रारंभिक क्वेरी निष्पादित करके शुरू करें। एकमात्र आवश्यकता यह है कि `id` को ORDER BY क्लॉज़ में शामिल किया जाना चाहिए ताकि लगातार पृष्ठन सुनिश्चित हो सके। क्वेरी आपके परिणामों और बाद के पृष्ठों के लिए एक स्क्रॉल टोकन दोनों लौटाएगी।

```sql
SELECT ... ORDER BY [... ,] id {ASC|DESC};
```

<!-- परिचय -->
उदाहरण:
<!-- अनुरोध प्रारंभिक क्वेरी उदाहरण -->
```sql
SELECT weight(), id FROM test WHERE match('hello') ORDER BY weight() desc, id asc limit 2;
```

<!-- प्रतिक्रिया प्रारंभिक क्वेरी उदाहरण -->
```sql
+----------+------+
| weight() | id   |
+----------+------+
|     1281 |    1 |
|     1281 |    2 |
+----------+------+
2 rows in set (0.00 sec)
```

<!-- अंत -->

<!-- उदाहरण scroll_sql_show -->
**स्क्रॉल टोकन प्राप्त करना**

प्रारंभिक क्वेरी को निष्पादित करने के बाद, `SHOW SCROLL` कमांड का उपयोग करके स्क्रॉल टोकन प्राप्त करें।

```sql
SHOW SCROLL;
```

प्रतिक्रिया:
```sql
| scroll_token                       |
|------------------------------------|
| <base64 encoded scroll token>      |
```

<!-- परिचय -->
उदाहरण:

<!-- अनुरोध स्क्रॉल टोकन उदाहरण -->
```sql
SHOW SCROLL;
```

<!-- प्रतिक्रिया स्क्रॉल टोकन उदाहरण -->
```sql
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| scroll_token                                                                                                                                                                                                             |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| eyJvcmRlcl9ieV9zdHIiOiJ3ZWlnaHQoKSBkZXNjLCBpZCBhc2MiLCJvcmRlcl9ieSI6W3siYXR0ciI6IndlaWdodCgpIiwiZGVzYyI6dHJ1ZSwidmFsdWUiOjEyODEsInR5cGUiOiJpbnQifSx7ImF0dHIiOiJpZCIsImRlc2MiOmZhbHNlLCJ2YWx1ZSI6MiwidHlwZSI6ImludCJ9XX0= |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- अंत -->

<!-- उदाहरण scroll_sql_paginated -->
**स्क्रॉल का उपयोग करके पृष्ठांकित क्वेरी**

 परिणामों के अगले पृष्ठ को प्राप्त करने के लिए, अगले क्वेरी में स्क्रॉल टोकन को एक विकल्प के रूप में शामिल करें। जब `scroll` विकल्प प्रदान किया जाता है, तो क्रमबद्धता मानदंड को निर्दिष्ट करना वैकल्पिक होता है।

```sql
SELECT ... [ORDER BY [... ,] id {ASC|DESC}] OPTION scroll='<base64 encoded scroll token>'[, ...];
```

यह सुनिश्चित करता है कि पृष्ठन निर्बाध रूप से जारी रहे, प्रारंभिक क्वेरी में स्थापित की गई क्रमबद्धता संदर्भ को बनाए रखते हुए।

<!-- परिचय -->
उदाहरण:

<!-- अनुरोध पृष्ठांकित क्वेरी उदाहरण -->
```sql
SELECT weight(), id FROM test WHERE match('hello') limit 2
OPTION scroll='eyJvcmRlcl9ieV9zdHIiOiJ3ZWlnaHQoKSBkZXNjLCBpZCBhc2MiLCJvcmRlcl9ieSI6W3siYXR0ciI6IndlaWdodCgpIiwiZGVzYyI6dHJ1ZSwidmFsdWUiOjEyODEsInR5cGUiOiJpbnQifSx7ImF0dHIiOiJpZCIsImRlc2MiOmZhbHNlLCJ2YWx1ZSI6MiwidHlwZSI6ImludCJ9XX0=';
```

<!-- प्रतिक्रिया पृष्ठांकित क्वेरी उदाहरण -->
```sql
+----------+------+
| weight() | id   |
+----------+------+
|     1281 |    3 |
|     1281 |    4 |
+----------+------+
2 rows in set (0.00 sec)
```

<!-- end -->

#### JSON के माध्यम से स्क्रॉलिंग

<!-- example scroll_json_init -->
**प्रारंभिक अनुरोध**

प्रारंभिक अनुरोध में, विकल्पों में `"scroll": true` और इच्छित क्रमबद्धता मानदंड निर्दिष्ट करें। ध्यान दें कि `id` को `sort` सूची में होना चाहिए। प्रतिक्रिया में स्क्रॉल टोकन शामिल होगा, जिसका उपयोग आगे के अनुरोधों में पेजिनेशन के लिए किया जा सकता है।

```json
POST /search
{ 
  "table": "<table_names>",
  "options": {

"scroll": true
  }, 
  
  ...
  
  "sort": [
    ...
    { "id":{ "order":"{asc|desc}"} }
  ]
}
```

उदाहरण आउटपुट:

```json
{
    "timed_out": false,
    "hits": {


...
    
    },
    "scroll": "<base64 encoded scroll token>"
}
```

<!-- request Initial Request Example -->
```json
POST /search
{ 
  "table": "test",
  "options":
  {

"scroll": true
  },
  "query":
  {  

"query_string":"hello"
  },
  "sort":
  [
    { "_score":{ "order":"desc"} },

{ "id":{ "order":"asc"} }
  ],
  "track_scores": true,
  "limit":2
}
```

<!-- response Initial Request Example -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits":
  {
    "total": 10,
    "total_relation": "eq",
    "hits":

[
      {
        "_id": 1,
        "_score": 1281,
        "_source":

{
          "title": "hello world1"
        }
      },
      {
        "_id": 2,
        "_score": 1281,
        "_source":

{
          "title": "hello world2"
        }
      }
    ]
  },
  "scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjoyLCJ0eXBlIjoiaW50In1dfQ=="
}
```
<!-- end -->

<!-- example scroll_json_paginated -->
**स्क्रॉल का उपयोग करते हुए पेजिनेटेड अनुरोध**

पेजिनेशन को जारी रखने के लिए, अगले अनुरोध के विकल्प ऑब्जेक्ट के भीतर पिछले प्रतिक्रिया से प्राप्त स्क्रॉल टोकन को शामिल करें। क्रमबद्धता मानदंड निर्दिष्ट करना वैकल्पिक है।

```json
POST /search
{ 
  "table": "<table_names>",
  "options": {
    "scroll": "<base64 encoded scroll token>"
  },

  ...

}
```

<!-- request Paginated Request Example -->

```json
POST /search
{ 
  "table": "test",
  "options":
  {

"scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjoyLCJ0eXBlIjoiaW50In1dfQ=="
  },
  "query":
  {  

"query_string":"hello"
  },
  "track_scores": true,
  "limit":2
}
```

<!-- response Paginated Request Example -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits":
  {
    "total": 8,
    "total_relation": "eq",
    "hits":
   [
      {
        "_id": 3,
        "_score": 1281,
        "_source":
        {
          "title": "hello world3"
        }
      },
      {
        "_id": 4,
        "_score": 1281,
        "_source":
        {
          "title": "hello world4"
# खोज परिणामों का पृष्ठीकरण

<!-- उदाहरण सामान्य -->

मंटिकोरे सर्च डिफ़ॉल्ट रूप से परिणाम सेट में शीर्ष 20 मेल खाने वाले दस्तावेज़ों को लौटाता है।

#### SQL
SQL में, आप `LIMIT` क्लॉज का उपयोग करके परिणाम सेट के माध्यम से नेविगेट कर सकते हैं।

`LIMIT` एक संख्या को स्वीकार कर सकता है जो लौटाए गए सेट के आकार के लिए शून्य ऑफ़्सेट के साथ है, या ऑफ़्सेट और आकार के मूल्यों के जोड़े को।

#### HTTP JSON
जब HTTP JSON का उपयोग करते हैं, तो नोड्स `offset` और `limit` परिणाम सेट के ऑफ़्सेट और लौटाए गए सेट के आकार को नियंत्रित करते हैं। वैकल्पिक रूप से, आप इसके बजाय `size` और `from` के जोड़े का उपयोग कर सकते हैं।

<!-- परिचय -->

<!-- अनुरोध SQL -->

```sql
SELECT  ... FROM ...  [LIMIT [offset,] row_count]
SELECT  ... FROM ...  [LIMIT row_count][ OFFSET offset]
```


<!-- अनुरोध JSON -->

```json
{
  "table": "<table_name>",
  "query": ...
  ...  
  "limit": 20,
  "offset": 0
}
{
  "table": "<table_name>",
  "query": ...
  ...  
  "size": 20,
  "from": 0
}
```

<!-- समाप्त -->

<!-- उदाहरण maxMatches -->
### परिणाम सेट विंडो

डिफ़ॉल्ट रूप से, मंटिकोरे सर्च 1000 सर्वश्रेष्ठ रैंक किए गए दस्तावेज़ों की परिणाम सेट विंडो का उपयोग करता है जो परिणाम सेट में लौटाए जा सकते हैं। यदि परिणाम सेट इस मान से परे पृष्ठबद्ध है, तो क्वेरी त्रुटि में समाप्त होगी।

इस सीमा को प्रश्न विकल्प [max_matches](../Searching/Options.md#max_matches) के साथ समायोजित किया जा सकता है।

`max_matches` को बहुत उच्च मूल्यों में बढ़ाना केवल तब किया जाना चाहिए जब यह नेविगेशन के लिए आवश्यक हो कि ऐसे बिंदुओं तक पहुंचा जा सके। एक उच्च `max_matches` मान अधिक मेमोरी की आवश्यकता करता है और क्वेरी प्रतिक्रिया समय को बढ़ा सकता है। गहरे परिणाम सेट के साथ काम करने का एक तरीका `max_matches` को ऑफ़्सेट और लिमिट का योग के रूप में सेट करना है।

`max_matches` को 1000 से नीचे कम करने का लाभ क्वेरी द्वारा उपयोग की जाने वाली मेमोरी को कम करना है। यह क्वेरी समय को भी कम कर सकता है, लेकिन अधिकांश मामलों में, यह एक ध्यान देने योग्य लाभ नहीं हो सकता है।

<!-- परिचय -->


<!-- अनुरोध SQL -->

```sql
SELECT  ... FROM ...   OPTION max_matches=<value>
```


<!-- अनुरोध JSON -->


```json
{
  "table": "<table_name>",
  "query": ...
  ...
  "max_matches":<value>
  }
}

```

<!-- समाप्त -->

# स्क्रॉल खोज विकल्प

स्क्रॉल खोज विकल्प बड़े परिणाम सेट के माध्यम से पृष्ठबद्ध करने का एक कुशल और विश्वसनीय तरीका प्रदान करता है। पारंपरिक ऑफ़्सेट-आधारित पृष्ठीकरण के विपरीत, स्क्रॉल खोज गहरे पृष्ठीकरण के लिए बेहतर प्रदर्शन प्रदान करता है और पृष्ठीकरण को लागू करने का एक आसान तरीका प्रदान करता है।

#### SQL के माध्यम से स्क्रॉलिंग

<!-- उदाहरण scroll_sql_init -->
**क्रमबद्धता मानदंड के साथ प्रारंभिक क्वेरी**

अपनी इच्छित क्रमबद्धता मानदंड के साथ एक प्रारंभिक क्वेरी निष्पादित करके प्रारंभ करें। केवल आवश्यकताएं हैं कि `id` को ORDER BY क्लॉज में शामिल किया जाना चाहिए ताकि लगातार पृष्ठीकरण सुनिश्चित हो सके। क्वेरी आपके परिणामों और आगे के पृष्ठों के लिए एक स्क्रॉल टोकन लौटाएगी।

```sql
SELECT ... ORDER BY [... ,] id {ASC|DESC};
```

<!-- परिचय -->
उदाहरण:
<!-- अनुरोध प्रारंभिक क्वेरी उदाहरण -->
```sql
SELECT weight(), id FROM test WHERE match('hello') ORDER BY weight() desc, id asc limit 2;
```

<!-- प्रतिक्रिया प्रारंभिक क्वेरी उदाहरण -->
```sql
+----------+------+
| weight() | id   |
+----------+------+
|     1281 |    1 |
|     1281 |    2 |
+----------+------+
2 rows in set (0.00 sec)
```

<!-- समाप्त -->

<!-- उदाहरण scroll_sql_show -->
**स्क्रॉल टोकन प्राप्त करना**

प्रारंभिक क्वेरी निष्पादित करने के बाद, `SHOW SCROLL` कमांड निष्पादित करके स्क्रॉल टोकन प्राप्त करें।

```sql
SHOW SCROLL;
```

प्रतिक्रिया:
```sql
| scroll_token                       |
|------------------------------------|
| <base64 encoded scroll token>      |
```

<!-- परिचय -->
उदाहरण:

<!-- अनुरोध स्क्रॉल टोकन उदाहरण -->
```sql
SHOW SCROLL;
```

<!-- प्रतिक्रिया स्क्रॉल टोकन उदाहरण -->
```sql
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| scroll_token                                                                                                                                                                                                             |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| eyJvcmRlcl9ieV9zdHIiOiJ3ZWlnaHQoKSBkZXNjLCBpZCBhc2MiLCJvcmRlcl9ieSI6W3siYXR0ciI6IndlaWdodCgpIiwiZGVzYyI6dHJ1ZSwidmFsdWUiOjEyODEsInR5cGUiOiJpbnQifSx7ImF0dHIiOiJpZCIsImRlc2MiOmZhbHNlLCJ2YWx1ZSI6MiwidHlwZSI6ImludCJ9XX0= |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- समाप्त -->

<!-- उदाहरण scroll_sql_paginated -->
**स्क्रॉल का उपयोग करके पृष्ठित क्वेरी**

 परिणामों के अगले पृष्ठ को प्राप्त करने के लिए, स्क्रॉल टोकन को अगले क्वेरी में एक विकल्प के रूप में शामिल करें। जब `scroll` विकल्प प्रदान किया जाता है, तो क्रमबद्धता मानदंड निर्दिष्ट करना वैकल्पिक होता है।

```sql
SELECT ... [ORDER BY [... ,] id {ASC|DESC}] OPTION scroll='<base64 encoded scroll token>'[, ...];
```

यह सुनिश्चित करता है कि पृष्ठीकरण सुचारू रूप से जारी रहे, प्रारंभिक क्वेरी में स्थापित क्रमबद्धता संदर्भ को बनाए रखते हुए।

<!-- परिचय -->
उदाहरण:

<!-- अनुरोध पृष्ठित क्वेरी उदाहरण -->
```sql
SELECT weight(), id FROM test WHERE match('hello') limit 2
OPTION scroll='eyJvcmRlcl9ieV9zdHIiOiJ3ZWlnaHQoKSBkZXNjLCBpZCBhc2MiLCJvcmRlcl9ieSI6W3siYXR0ciI6IndlaWdodCgpIiwiZGVzYyI6dHJ1ZSwidmFsdWUiOjEyODEsInR5cGUiOiJpbnQifSx7ImF0dHIiOiJpZCIsImRlc2MiOmZhbHNlLCJ2YWx1ZSI6MiwidHlwZSI6ImludCJ9XX0=';
```

<!-- प्रतिक्रिया पृष्ठित क्वेरी उदाहरण -->
```sql
+----------+------+
| वजन()   | आईडी   |
+----------+------+
|     1281 |    3 |
|     1281 |    4 |
+----------+------+
2 पंक्तियाँ सेट में (0.00 सेकंड)
```

<!-- end -->

#### JSON के माध्यम से स्क्रॉलिंग

<!-- example scroll_json_init -->
**प्रारंभिक अनुरोध**

प्रारंभिक अनुरोध में, विकल्पों में `"scroll": true` निर्दिष्ट करें और इच्छित क्रम निर्धारण मानदंड। ध्यान दें कि `id` को `sort` सूची में होना चाहिए। प्रतिक्रिया में एक स्क्रॉल टोकन शामिल होगा, जिसका उपयोग आगामी अनुरोधों में पृष्ठांकन के लिए किया जा सकता है।

```json
POST /search
{ 
  "table": "<table_names>",
  "options": {
	  "scroll": true
  }, 
  
  ...
  
  "sort": [
    ...
    { "id":{ "order":"{asc|desc}"} }
  ]
}
```

उदाहरण आउटपुट:

```json
{
    "timed_out": false,
    "hits": {

		...
    
    },
    "scroll": "<base64 encoded scroll token>"
}
```

<!-- request Initial Request Example -->
```json
POST /search
{ 
  "table": "test",
  "options":
  {
	"scroll": true
  },
  "query":
  {  
	"query_string":"hello"
  },
  "sort":
  [
    { "_score":{ "order":"desc"} },
	{ "id":{ "order":"asc"} }
  ],
  "track_scores": true,
  "limit":2
}
```

<!-- response Initial Request Example -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits":
  {
    "total": 10,
    "total_relation": "eq",
    "hits":
	[
      {
        "_id": 1,
        "_score": 1281,
        "_source":
		{
          "title": "hello world1"
        }
      },
      {
        "_id": 2,
        "_score": 1281,
        "_source":
		{
          "title": "hello world2"
        }
      }
    ]
  },
  "scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjoyLCJ0eXBlIjoiaW50In1dfQ=="
}
```
<!-- end -->

<!-- example scroll_json_paginated -->
**स्क्रॉल का उपयोग करके पृष्ठांकित अनुरोध**

पृष्ठांकन जारी रखने के लिए, अगले अनुरोध के विकल्प वस्तु के भीतर पिछले प्रतिक्रिया से प्राप्त स्क्रॉल टोकन शामिल करें। क्रम निर्धारण मानदंड निर्दिष्ट करना वैकल्पिक है।

```json
POST /search
{ 
  "table": "<table_names>",
  "options": {
    "scroll": "<base64 encoded scroll token>"
  },

  ...

}
```

<!-- request Paginated Request Example -->

```json
POST /search
{ 
  "table": "test",
  "options":
  {
	"scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjoyLCJ0eXBlIjoiaW50In1dfQ=="
  },
  "query":
  {  
	"query_string":"hello"
  },
  "track_scores": true,
  "limit":2
}
```

<!-- response Paginated Request Example -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits":
  {
    "total": 8,
    "total_relation": "eq",
    "hits":
   [
      {
        "_id": 3,
        "_score": 1281,
        "_source":
        {
          "title": "hello world3"
        }
      },
      {
        "_id": 4,
        "_score": 1281,
        "_source":
        {
          "title": "hello world4"
        }
      }
    ]
  },
  "scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjo0LCJ0eXBlIjoiaW50In1dfQ=="
}
```
<!-- end -->

<!-- proofread -->
