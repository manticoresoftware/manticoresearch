# HTTP

आप HTTP/HTTPS के माध्यम से Manticore Search से कनेक्ट कर सकते हैं।

## कॉन्फ़िगरेशन
<!-- example HTTP -->
डिफ़ॉल्ट रूप से, Manticore HTTP, HTTPS, और बाइनरी अनुरोधों के लिए पोर्ट 9308 और 9312 पर सुनता है।

अपने कॉन्फ़िगरेशन फ़ाइल के "searchd" अनुभाग में, आप `listen` निर्देश का उपयोग करके HTTP पोर्ट को निम्नलिखित के रूप में परिभाषित कर सकते हैं:

दोनों पंक्तियाँ मान्य हैं और इसका वही अर्थ है (पोर्ट नंबर के अपवाद के साथ)। यह दोनों सभी API/HTTP/HTTPS प्रोटोकॉल के लिए सर्वर की सर्विस करने वाले श्रोताओं को परिभाषित करते हैं। कोई विशेष आवश्यकताएँ नहीं हैं, और Manticore से कनेक्ट करने के लिए किसी भी HTTP क्लाइंट का उपयोग किया जा सकता है।

<!-- request HTTP -->
```ini
searchd {
...
   listen = 127.0.0.1:9308
   listen = 127.0.0.1:9312:http
...
}
```
<!-- end -->

सभी HTTP एंडपॉइंट `application/json` सामग्री प्रकार लौटाते हैं। अधिकांश भाग के लिए, एंडपॉइंट अनुरोधों के लिए JSON पेलोड का उपयोग करते हैं। हालांकि, कुछ अपवाद हैं जो NDJSON या सरल URL-encoded पेलोड का उपयोग करते हैं।

वर्तमान में, कोई उपयोगकर्ता प्रमाणीकरण नहीं है। इसलिए, सुनिश्चित करें कि HTTP इंटरफ़ेस आपके नेटवर्क के बाहर किसी के लिए सुलभ नहीं है। चूंकि Manticore किसी अन्य वेब सर्वर की तरह कार्य करता है, आप HTTP प्रमाणीकरण या कैशिंग को लागू करने के लिए Nginx जैसे रिवर्स प्रॉक्सी का उपयोग कर सकते हैं।

<!-- example HTTPS -->
HTTP प्रोटोकॉल [SSL एन्क्रिप्शन](../Security/SSL.md) का भी समर्थन करता है:
यदि आप `:http` के बजाय `:https` निर्दिष्ट करते हैं तो **केवल** सुरक्षित कनेक्शन स्वीकार किए जाएंगे। अन्यथा, यदि कोई मान्य कुंजी/प्रमाणपत्र प्रदान नहीं किया गया है, लेकिन क्लाइंट HTTPS के माध्यम से कनेक्ट करने की कोशिश कर रहा है - तो कनेक्शन को गिरा दिया जाएगा। यदि आप 9443 पर HTTPS नहीं, बल्कि HTTP अनुरोध करते हैं तो यह HTTP कोड 400 के साथ प्रतिक्रिया करेगा।

<!-- request HTTPS -->
```ini
searchd {
...
   listen = 127.0.0.1:9308
   listen = 127.0.0.1:9443:https
...
}
```
<!-- end -->

### VIP कनेक्शन
<!-- example VIP -->
'VIP' कनेक्शनों के लिए अलग HTTP इंटरफ़ेस का उपयोग किया जा सकता है। इस मामले में, कनेक्शन एक थ्रेड पूल को बाईपास करता है और हमेशा एक नया समर्पित थ्रेड बनाता है। यह Manticore Search को गंभीर ओवरलोड के दौरान प्रबंधित करने के लिए उपयोगी है जब सर्वर रुक सकता है या नियमित पोर्ट कनेक्शनों की अनुमति नहीं दे सकता है।

`listen` निर्देश के बारे में अधिक जानकारी के लिए, [इस अनुभाग](../Server_settings/Searchd.md#listen) को देखें।

<!-- request VIP -->
```ini
searchd {
...
   listen = 127.0.0.1:9308
   listen = 127.0.0.1:9318:_vip
...
}
```
<!-- end -->

## HTTP पर SQL

Manticore HTTP के माध्यम से SQL क्वेरी चलाने के लिए `/sql`, `/cli`, और `/cli_json` एंडपॉइंट प्रदान करता है। प्रत्येक एंडपॉइंट विशेष उपयोग के मामलों के लिए डिज़ाइन किया गया है:

* `/sql`: अनुप्रयोगों से प्रोग्रामेटिक उपयोग के लिए उपयुक्त।
  - `/sql` एंडपॉइंट केवल SELECT कथनों को स्वीकार करता है और HTTP JSON प्रारूप में प्रतिक्रिया लौटाता है।
  - `/sql?mode=raw` एंडपॉइंट कोई भी SQL क्वेरी स्वीकार करता है और कच्चे प्रारूप में प्रतिक्रिया लौटाता है, जो आपको mysql के माध्यम से प्राप्त होगा।
* `/cli`: **केवल मैनुअल उपयोग** के लिए अभिप्रेत (जैसे, curl या ब्राउज़र के माध्यम से)। **स्क्रिप्ट के लिए अनुशंसित नहीं।**
* `/cli_json`: `/cli` के समान, लेकिन JSON प्रारूप में परिणाम लौटाता है। **स्क्रिप्ट के लिए अनुशंसित नहीं।**

### /sql

<!-- example SQL_over_HTTP -->

सामान्य वाक्यSyntax:
* `curl "localhost:6780/sql[?mode=raw]&query={URL_ENCODED_QUERY}"`
* `curl localhost:6780/sql[?mode=raw] -d "[query={URL_ENCODED_QUERY}|{NOT_URL_ENCODED_QUERY}]"`

`/sql` एंडपॉइंट HTTP JSON इंटरफ़ेस के माध्यम से SQL क्वेरी को स्वीकार करता है:
* `mode=raw` के बिना - केवल [SELECTs](../Searching/Full_text_matching/Basic_usage.md#SQL) की अनुमति है, जो JSON प्रारूप में प्रतिक्रिया लौटाता है।
* [mode=raw](../Connecting_to_the_server/HTTP.md#mode=raw) के साथ - कोई भी SQL क्वेरी की अनुमति है, जो कच्चे प्रारूप में प्रतिक्रिया लौटाती है।

एंडपॉइंट HTTP अनुरोधों को GET या POST विधि का उपयोग करके संभाल सकता है। क्वेरी भेजने के लिए, आप:
1. **GET का उपयोग करते हुए:** URL के `query` पैरामीटर में क्वेरी को शामिल करें, जैसे `/sql?query=your_encoded_query_here`। इस पैरामीटर को त्रुटियों से बचने के लिए **URL एन्कोड करना महत्वपूर्ण है**, विशेष रूप से यदि क्वेरी में '=' संकेत शामिल है, जिसे URL व्याकरण का हिस्सा माना जा सकता है न कि क्वेरी का।
2. **POST का उपयोग करते हुए:** आप POST अनुरोध के शरीर के भीतर क्वेरी भी भेज सकते हैं। इस विधि का उपयोग करते समय:
   - यदि आप क्वेरी को `query` नामक पैरामीटर के रूप में भेजते हैं, तो **सुनिश्चित करें कि यह URL एन्कोडेड है**।
   - यदि आप क्वेरी को सीधे स्पष्ट पाठ (कच्चे POST शरीर) के रूप में भेजते हैं, तो **इसे URL एन्कोड नहीं करें**। जब क्वेरी लंबी या जटिल होती है, या यदि क्वेरी एक फ़ाइल में संग्रहीत होती है और आप इसे जैसे ही भेजना चाहते हैं, तो यह उपयोगी है।

यह दृष्टिकोण GET और POST के उपयोग को भिन्न बनाए रखता है और एकल अनुरोध में विधियों को संयोजित करने के बारे में किसी भी भ्रम से बचता है।

`mode=raw` के बिना प्रतिक्रिया एक JSON होती है जिसमें हिट और निष्पादन समय के बारे में जानकारी होती है। प्रतिक्रिया प्रारूप वही है जैसा कि [json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) एंडपॉइंट में है। ध्यान दें कि `/sql` एंडपॉइंट केवल एकल खोज अनुरोधों का समर्थन करता है। मल्टी-क्वेरी प्रोसेसिंग के लिए, नीचे दिए गए [कच्चे मोड](../Connecting_to_the_server/HTTP.md#mode=raw) अनुभाग को देखें।

<!-- request POST -->
```bash
POST /sql
select id,subject,author_id  from forum where match('@subject php manticore') group by author_id order by id desc limit 0,5
```

<!-- response POST -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 2,
        "_score": 2356,
        "_source": {
          "subject": "php manticore",
          "author_id": 12
        }
      },
      {
        "_id": 1,
        "_score": 2356,
        "_source": {
          "subject": "php manticore",
          "author_id": 11
        }
      }
    ]
  }
}
```

<!-- request POST URL-encoded -->
```bash
POST /sql query=select%20id%2Csubject%2Cauthor_id%20%20from%20forum%20where%20match%28%27%40subject%20php%20manticore%27%29%20group%20by%20author_id%20order%20by%20id%20desc%20limit%200%2C5
```

<!-- response POST URL-encoded -->
```json
{

  "took": 0,

  "timed_out": false,

  "hits": {

    "total": 2,

    "total_relation": "eq",

    "hits": [

      {

        "_id": 2,

        "_score": 2356,

        "_source": {

          "subject": "php manticore",

          "author_id": 12

        }

      },

      {

        "_id": 1,

        "_score": 2356,

        "_source": {

          "subject": "php manticore",

          "author_id": 11

        }

      }

    ]

  }

}
```

<!-- request GET URL-encoded -->
```bash
GET /sql?query=select%20id%2Csubject%2Cauthor_id%20%20from%20forum%20where%20match%28%27%40subject%20php%20manticore%27%29%20group%20by%20author_id%20order%20by%20id%20desc%20limit%200%2C5
```

<!-- response GET URL-encoded -->
```json
{

  "took": 0,

  "timed_out": false,

  "hits": {

    "total": 2,

    "total_relation": "eq",

    "hits": [

      {

        "_id": 2,

        "_score": 2356,

        "_source": {

          "subject": "php manticore",

          "author_id": 12

        }

      },

      {

        "_id": 1,

        "_score": 2356,

        "_source": {

          "subject": "php manticore",

          "author_id": 11

        }

      }

    ]

  }

}
```

<!-- end -->

#### mode=raw

<!-- example mode=raw -->

`/sql` एंडपॉइंट में एक विशेष "raw" मोड भी शामिल है, जो आपको **कोई भी मान्य SQL क्वेरी, मल्टी-क्वेरीज़ सहित** भेजने की अनुमति देता है। प्रतिक्रिया एक JSON एरे होती है जिसमें एक या अधिक परिणाम सेट होते हैं। आप विकल्प `mode=raw` का उपयोग करके इस मोड को सक्रिय कर सकते हैं।

<!-- request POST -->
```bash
POST /sql?mode=raw
desc test
```

<!-- response POST -->
```json
[
  {

    "columns": [

      {

        "Field": {

          "type": "string"

        }

      },

      {

        "Type": {

          "type": "string"

        }

      },

      {

        "Properties": {

          "type": "string"

        }

      }

    ],

    "data": [

      {

        "Field": "id",

        "Type": "bigint",

        "Properties": ""

      },

      {

        "Field": "title",

        "Type": "text",

        "Properties": "indexed"

      },

      {

        "Field": "gid",

        "Type": "uint",

        "Properties": ""

      },

      {

        "Field": "title",

        "Type": "string",

        "Properties": ""

      },

      {

        "Field": "j",

        "Type": "json",

        "Properties": ""

      },

      {

        "Field": "new1",

        "Type": "uint",

        "Properties": ""

      }

    ],

    "total": 6,

    "error": "",

    "warning": ""

  }
]
```

<!-- request POST URL-encoded -->
```bash
POST /sql?mode=raw
query=desc%20test
```

<!-- response POST URL-encoded -->
```json
[
  {

    "columns": [

      {

        "Field": {

          "type": "string"

        }

      },

      {

        "Type": {

          "type": "string"

        }

      },

      {

        "Properties": {

          "type": "string"

        }

      }

    ],

    "data": [

      {

        "Field": "id",

        "Type": "bigint",

        "Properties": ""

      },

      {

        "Field": "title",

        "Type": "text",

        "Properties": "indexed"

      },

      {

        "Field": "gid",

        "Type": "uint",

        "Properties": ""

      },

      {

        "Field": "title",

        "Type": "string",

        "Properties": ""

      },

      {

        "Field": "j",

        "Type": "json",

        "Properties": ""

      },

      {

        "Field": "new1",

        "Type": "uint",

        "Properties": ""

      }

    ],

    "total": 6,

    "error": "",

    "warning": ""

  }
]
```

<!-- request POST URL-encoded 2nd way -->
```bash
POST /sql
mode=raw&query=desc%20test
```

<!-- response POST URL-encoded 2nd way -->
```json
[
  {

    "columns": [

      {

        "Field": {

          "type": "string"

        }

      },

      {

        "Type": {

          "type": "string"

        }

      },

      {

        "Properties": {

          "type": "string"

        }

      }

    ],

    "data": [

      {

        "Field": "id",

        "Type": "bigint",

        "Properties": ""

      },

      {

        "Field": "title",

        "Type": "text",

        "Properties": "indexed"

      },

      {

        "Field": "gid",

        "Type": "uint",

        "Properties": ""

      },

      {

        "Field": "title",

        "Type": "string",

        "Properties": ""

      },

      {

        "Field": "j",

        "Type": "json",

        "Properties": ""

      },

      {

        "Field": "new1",

        "Type": "uint",

        "Properties": ""

      }

    ],

    "total": 6,

    "error": "",

    "warning": ""

  }
]
```

<!-- request GET URL-encoded -->
```bash
GET /sql?mode=raw&query=desc%20test
```

<!-- response GET URL-encoded -->
```json
[
  {

    "columns": [

      {

        "Field": {

          "type": "string"

        }

      },

      {

        "Type": {

          "type": "string"

        }

      },

      {

        "Properties": {

          "type": "string"

        }

      }

    ],

    "data": [

      {

        "Field": "id",

        "Type": "bigint",

        "Properties": ""

      },
      {
        "Field": "title",
        "Type": "text",
        "Properties": "indexed"
      },
      {
        "Field": "gid",
        "Type": "uint",
        "Properties": ""
      },
      {
        "Field": "title",
        "Type": "string",
        "Properties": ""
      },
      {
        "Field": "j",
        "Type": "json",
        "Properties": ""
      },
      {
        "Field": "new1",
        "Type": "uint",
        "Properties": ""
      }
    ],
    "total": 6,
    "error": "",
    "warning": ""
  }
]
```

<!-- request curl examples -->
```bash
