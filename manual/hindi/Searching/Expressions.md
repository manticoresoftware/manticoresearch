# खोज में अभिव्यक्तियाँ

Manticore SQL और HTTP दोनों के माध्यम से मनमानी अंकगणितीय अभिव्यक्तियों का उपयोग सक्षम करता है, जिसमें विशेषता मान, आंतरिक विशेषताएँ (दस्तावेज़ आईडी और प्रासंगिकता भार), अंकगणितीय संचालन, कई अंतर्निर्मित कार्य और उपयोगकर्ता-परिभाषित कार्य शामिल हैं। त्वरित पहुँच के लिए यहाँ पूर्ण संदर्भ सूची है।

## अंकगणितीय ऑपरेटर
```sql
+, -, *, /, %, DIV, MOD
```

मानक अंकगणितीय ऑपरेटर उपलब्ध हैं। इन ऑपरेटरों से संबंधित अंकगणितीय गणनाएँ तीन विभिन्न मोड में की जा सकती हैं:

1. एकल-प्रिसिजन, 32-बिट IEEE 754 फ्लोटिंग पॉइंट मानों का उपयोग करते हुए (डिफ़ॉल्ट),
2. साइन किए गए 32-बिट पूर्णांक का उपयोग करते हुए,
3. 64-बिट साइन किए गए पूर्णांक का उपयोग करते हुए।

यदि कोई संचालन फ्लोटिंग पॉइंट मान में परिणाम नहीं देता है, तो अभिव्यक्ति पार्सर स्वचालित रूप से पूर्णांक मोड में स्विच कर जाता है। अन्यथा, यह डिफ़ॉल्ट फ्लोटिंग पॉइंट मोड का उपयोग करता है। उदाहरण के लिए, a+b को 32-बिट पूर्णांकों का उपयोग करते हुए गणना की जाएगी यदि दोनों तर्क 32-बिट पूर्णांक हैं; या 64-बिट पूर्णांकों का उपयोग करते हुए यदि दोनों तर्क पूर्णांक हैं लेकिन उनमें से एक 64-बिट है; या अन्यथा फ्लोट में। हालाँकि, `a/b` या `sqrt(a)` हमेशा फ्लोट में गणना की जाएगी, क्योंकि ये संचालन गैर-पूर्णांक परिणाम लौटाते हैं। इसे टालने के लिए, आप `IDIV(a,b)` या `DIV b` रूप का उपयोग कर सकते हैं। इसके अतिरिक्त, `a*b` स्वतः 64-बिट में प्रोमोट नहीं होगा जब तर्क 32-बिट हों। 64-बिट परिणामों को लागू करने के लिए, [BIGINT()](../Functions/Type_casting_functions.md#BIGINT%28%29) का उपयोग करें, लेकिन ध्यान दें कि यदि गैर-पूर्णांक संचालन मौजूद हैं, तो BIGINT() को बस नजरअंदाज कर दिया जाएगा।

## तुलना ऑपरेटर
```sql
<, > <=, >=, =, <>
```

तुलना ऑपरेटर 1.0 लौटाते हैं जब शर्त सत्य होती है और अन्यथा 0.0। उदाहरण के लिए, `(a=b)+3` तब 4 का मूल्यांकन करता है जब विशेषता `a` विशेषता `b` के समान होती है, और 3 का मूल्यांकन करता है जब `a` असमान होता है। MySQL के विपरीत, समानता तुलना (यानी, `=` और `<>` ऑपरेटर) में एक छोटी समानता सीमा (डिफ़ॉल्ट रूप से 1e-6) शामिल होती है। यदि तुलना की गई मानों के बीच का अंतर सीमा के भीतर है, तो उन्हें समान माना जाता है।

कई मान वाले विशेषताओं के मामले में `BETWEEN` और `IN` ऑपरेटर सत्य लौटाते हैं यदि कम से कम एक मान स्थिति से मेल खाता है ( [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29) के समान)। `IN` ऑपरेटर JSON विशेषताओं का समर्थन नहीं करता है। `IS (NOT) NULL` ऑपरेटर केवल JSON विशेषताओं के लिए समर्थित है।

## बूलियन ऑपरेटर
```sql
AND, OR, NOT
```

बूलियन ऑपरेटर (AND, OR, NOT) अपेक्षित व्यवहार करते हैं। वे बाएँ-एसोसिएटिव हैं और अन्य ऑपरेटरों की तुलना में सबसे कम प्राथमिकता रखते हैं। NOT का AND और OR की तुलना में अधिक प्राथमिकता है लेकिन किसी अन्य ऑपरेटर की तुलना में अभी भी कम है। AND और OR समान प्राथमिकता साझा करते हैं, इसलिए जटिल अभिव्यक्तियों में भ्रम से बचने के लिए कोष्ठकों का उपयोग करने की सिफारिश की जाती है।

## बिटवाइज ऑपरेटर
```sql
&, |
```

ये ऑपरेटर क्रमशः बिटवाइज AND और OR करते हैं। ऑपरेटर पूर्णांक प्रकार के होने चाहिए।

## कार्य:
* [ABS()](../Functions/Mathematical_functions.md#ABS%28%29)
* [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29)
* [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)
* [ATAN2()](../Functions/Mathematical_functions.md#ATAN2%28%29)
* [BIGINT()](../Functions/Type_casting_functions.md#BIGINT%28%29)
* [BITDOT()](../Functions/Mathematical_functions.md#BITDOT%28%29)
* [BM25F()](../Functions/Searching_and_ranking_functions.md#BM25F%28%29)
* [CEIL()](../Functions/Mathematical_functions.md#CEIL%28%29)
* [CONCAT()](../Functions/String_functions.md#CONCAT%28%29)
* [CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29)
* [COS()](../Functions/Mathematical_functions.md#COS%28%29)
* [CRC32()](../Functions/Mathematical_functions.md#CRC32%28%29)
* [DATE_HISTOGRAM()](../Functions/Date_and_time_functions.md#DATE_HISTOGRAM%28%29)
* [DATE_RANGE()](../Functions/Date_and_time_functions.md#DATE_RANGE%28%29)
* [DAY()](../Functions/Date_and_time_functions.md#DAY%28%29)
* [DOUBLE()](../Functions/Type_casting_functions.md#DOUBLE%28%29)
* [EXP()](../Functions/Mathematical_functions.md#EXP%28%29)
* [FIBONACCI()](../Functions/Mathematical_functions.md#FIBONACCI%28%29)
* [FLOOR()](../Functions/Mathematical_functions.md#FLOOR%28%29)
* [GEODIST()](../Functions/Geo_spatial_functions.md#GEODIST%28%29)
* [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29)
* [GREATEST()](../Functions/Mathematical_functions.md#GREATEST%28%29)
* [HOUR()](../Functions/Date_and_time_functions.md#HOUR%28%29)
* [HISTOGRAM()](../Functions/Arrays_and_conditions_functions.md#HISTOGRAM%28%29)
* [IDIV()](../Functions/Mathematical_functions.md#IDIV%28%29)
* [IF()](../Functions/Arrays_and_conditions_functions.md#IF%28%29)
* [IN()](../Functions/Arrays_and_conditions_functions.md#IN%28%29)
* [INDEXOF()](../Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29)
* [INTEGER()](../Functions/Type_casting_functions.md#INTEGER%28%29)
* [INTERVAL()](../Functions/Arrays_and_conditions_functions.md#INTERVAL%28%29)
* [LAST_INSERT_ID()](../Functions/Other_functions.md#LAST_INSERT_ID%28%29)
* [LEAST()](../Functions/Mathematical_functions.md#LEAST%28%29)
* [LENGTH()](../Functions/Arrays_and_conditions_functions.md#LENGTH%28%29)
* [LN()](../Functions/Mathematical_functions.md#LN%28%29)
* [LOG10()](../Functions/Mathematical_functions.md#LOG10%28%29)
* [LOG2()](../Functions/Mathematical_functions.md#LOG2%28%29)
* [MAX()](../Functions/Mathematical_functions.md#MAX%28%29)
* [MIN()](../Functions/Mathematical_functions.md#MIN%28%29)
* [MINUTE()](../Functions/Date_and_time_functions.md#MINUTE%28%29)
* [MIN_TOP_SORTVAL()](../Functions/Searching_and_ranking_functions.md#MIN_TOP_SORTVAL%28%29)
* [MIN_TOP_WEIGHT()](../Functions/Searching_and_ranking_functions.md#MIN_TOP_WEIGHT%28%29)
* [MONTH()](../Functions/Date_and_time_functions.md#MONTH%28%29)
* [NOW()](../Functions/Date_and_time_functions.md#NOW%28%29)
* [PACKEDFACTORS()](../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29)
* [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29)
* [POW()](../Functions/Mathematical_functions.md#POW%28%29)
* [RAND()](../Functions/Mathematical_functions.md#RAND%28%29)
* [RANGE()](../Functions/Arrays_and_conditions_functions.md#RANGE%28%29)
* [REGEX()](../Functions/String_functions.md#REGEX%28%29)
* [REMAP()](../Functions/Arrays_and_conditions_functions.md#REMAP%28%29)
* [SECOND()](../Functions/Date_and_time_functions.md#SECOND%28%29)
* [SIN()](../Functions/Mathematical_functions.md#SIN%28%29)
* [SINT()](../Functions/Type_casting_functions.md#SINT%28%29)
* [SQRT()](../Functions/Mathematical_functions.md#SQRT%28%29)
* [SUBSTRING_INDEX()](../Functions/String_functions.md#SUBSTRING_INDEX%28%29)
* [TO_STRING()](../Functions/Type_casting_functions.md#TO_STRING%28%29)
* [UINT()](../Functions/Type_casting_functions.md#UINT%28%29)
* [YEAR()](../Functions/Date_and_time_functions.md#YEAR%28%29)
* [YEARMONTH()](../Functions/Date_and_time_functions.md#YEARMONTH%28%29)
* [YEARMONTHDAY()](../Functions/Date_and_time_functions.md#YEARMONTHDAY%28%29)
* [WEIGHT()](../Functions/Searching_and_ranking_functions.md#WEIGHT%28%29)

## HTTP JSON में अभिव्यक्तियाँ

HTTP JSON इंटरफ़ेस में, अभिव्यक्तियों का समर्थन `script_fields` और `expressions` के माध्यम से किया जाता है।

### script_fields

<!-- example script_fields -->
```json
{
	"table": "test",
	"query": {
		"match_all": {}
	}, "script_fields": {
		"add_all": {
			"script": {
				"inline": "( gid * 10 ) | crc32(title)"
			}
		},
		"title_len": {
			"script": {
	   			"inline": "crc32(title)"
			}
		}
	}
}
```

इस उदाहरण में, दो अभिव्यक्तियाँ बनाई गई हैं: `add_all` और `title_len`। पहली अभिव्यक्ति `( gid * 10 ) | crc32(title)` की गणना करती है और परिणाम को `add_all` विशेषता में सुरक्षित करती है। दूसरी अभिव्यक्ति `crc32(title)` की गणना करती है और परिणाम को `title_len` विशेषता में सुरक्षित करती है।

वर्तमान में, केवल `inline` अभिव्यक्तियों का समर्थन किया जाता है। `inline` प्रॉपर्टी का मान (जिसे गणना करने के लिए अभिव्यक्ति कहा जाता है) SQL अभिव्यक्तियों की समान सिंटैक्स होती है।

अभिव्यक्ति का नाम छानने या सॉर्ट करने में उपयोग किया जा सकता है।


<!-- intro -->
##### script_fields:

<!-- request script_fields -->
```json
{
	"table":"movies_rt",
	"script_fields":{
		"cond1":{
			"script":{
				"inline":"actor_2_facebook_likes =296 OR movie_facebook_likes =37000"
			}
		},
		"cond2":{
			"script":{
				"inline":"IF (IN (content_rating,'TV-PG','PG'),2, IF(IN(content_rating,'TV-14','PG-13'),1,0))"
			}
		}
	},
	"limit":10,
	"sort":[
		{
			"cond2":"desc"
		},
		{
			"actor_1_name":"asc"
		},
		{
			"actor_2_name":"desc"
		}
	],
	"profile":true,
	"query":{
		"bool":{
			"must":[
				{
					"match":{
						"*":"star"
					}
				},
				{
					"equals":{
						"cond1":1
					}
				}
			],
			"must_not":[
				{
					"equals":{
						"content_rating":"R"
					}
				}
			]
		}
	}
}
```

<!-- end -->

डिफ़ॉल्ट रूप से, अभिव्यक्ति मान `_source` एरे में परिणाम सेट में शामिल होते हैं। यदि स्रोत चयनात्मक है (देखें [Source selection](../Searching/Search_results.md#Source-selection)), तो अभिव्यक्ति नाम को `_source` पैरामीटर में जोड़ा जा सकता है। ध्यान दें, अभिव्यक्तियों के नाम छोटे अक्षरों में होने चाहिए।

### expressions

<!-- example expressions -->

`expressions` एक सरल सिंटैक्स के साथ `script_fields` का एक विकल्प है। उदाहरण अनुरोध दो अभिव्यक्तियाँ जोड़ता है और परिणामों को `add_all` और `title_len` विशेषताओं में सुरक्षित करता है। ध्यान दें, अभिव्यक्तियों के नाम छोटे अक्षरों में होने चाहिए।

<!-- request expressions -->
```json
{
  "table": "test",
  "query": { "match_all": {} },
  "expressions":
  {
	  "add_all": "( gid * 10 ) | crc32(title)",
      "title_len": "crc32(title)"
  }
}
```

<!-- end -->

<!-- proofread -->
