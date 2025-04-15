# स्ट्रिंग फ़ंक्शन

### CONCAT()
दो या एक से अधिक स्ट्रिंग को एक में संकुचित करता है। गैर-स्ट्रिंग तर्कों को `TO_STRING()` फ़ंक्शन का उपयोग करके स्पष्ट रूप से स्ट्रिंग में परिवर्तित किया जाना चाहिए।

```sql
CONCAT(TO_STRING(float_attr), ',', TO_STRING(int_attr), ',', title)
```

### LEVENSHTEIN()
`LEVENSHTEIN ( pattern, source, {normalize=0, length_delta=0})` संख्या (लेवेन्सhtein दूरी) लौटाता है जो `pattern` और `source` स्ट्रिंग में एकल-चरित्र संपादनों (सम्मिलन, विलोपन या प्रतिस्थापन) की संख्या है जो `pattern` में होने की आवश्यकता है ताकि इसे `source` बनाया जा सके।

   * `pattern`, `source` - स्थिर स्ट्रिंग, स्ट्रिंग फ़ील्ड नाम, JSON फ़ील्ड नाम, या कोई भी अभिव्यक्ति जो एक स्ट्रिंग उत्पन्न करती है (जैसे [SUBSTRING_INDEX()](../Functions/String_functions.md#SUBSTRING_INDEX%28%29))
   * `normalize` - विकल्प जो दूरी को `[0.0 - 1.0]` रेंज में एक बिंदुवादी संख्या के रूप में लौटाता है, जहाँ 0.0 एक सटीक मेल है, और 1.0 अधिकतम अंतर है। डिफ़ॉल्ट मान 0 है, जिसका अर्थ है सामान्यीकृत न करना और परिणाम को पूर्णांक के रूप में प्रदान करना।
   * `length_delta` - लेवेन्सhtein दूरी की गणना को छोड़ देता है और यदि विकल्प सेट किया गया है और स्ट्रिंग की लंबाई `length_delta` मान से अधिक भिन्न होती है तो `max(strlen(pattern), strlen(source))` लौटाता है। डिफ़ॉल्ट मान 0 है, जिसका अर्थ है किसी भी इनपुट स्ट्रिंग के लिए लेवेन्सhtein दूरी की गणना करना। यह विकल्प मुख्यतः समान स्ट्रिंग की जांच करते समय उपयोगी हो सकता है।

``` sql
SELECT LEVENSHTEIN('gily', attr1) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC, dist ASC;
SELECT LEVENSHTEIN('gily', j.name, {length_delta=6}) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC;
SELECT LEVENSHTEIN(title, j.name, {normalize=1}) AS dist, WEIGHT() AS w FROM test WHERE MATCH ('test') ORDER BY w DESC, dist ASC;
```

### REGEX()
`REGEX(attr,expr)` फ़ंक्शन 1 लौटाता है यदि कोई नियमित अभिव्यक्ति विशेषता की स्ट्रिंग से मेल खाती है, और अन्यथा 0। यह दोनों स्ट्रिंग और JSON विशेषताओं के साथ काम करता है।

```sql
SELECT REGEX(content, 'box?') FROM test;
SELECT REGEX(j.color, 'red | pink') FROM test;
```

अभिव्यक्तियों को RE2 सिंटैक्स का पालन करना चाहिए। उदाहरण के लिए, केस-इंसेंसिटिव सर्च करने के लिए, आप इसका उपयोग कर सकते हैं:
```sql
SELECT REGEX(content, '(?i)box') FROM test;
```

### SNIPPET()
`SNIPPET()` फ़ंक्शन का उपयोग दिए गए टेक्स्ट के भीतर खोज परिणामों को उजागर करने के लिए किया जा सकता है। पहले दो तर्क हैं: उजागर किया जाने वाला टेक्स्ट, और एक क्वेरी। [विकल्प](../Searching/Highlighting.md#Highlighting-options) को तीसरे, चौथे, और आगे के तर्क के रूप में फ़ंक्शन में दिया जा सकता है। `SNIPPET()` डिजिट से सीधे उजागर करने के लिए टेक्स्ट प्राप्त कर सकता है। इस मामले में, पहला तर्क फ़ील्ड नाम होना चाहिए:

```sql
SELECT SNIPPET(body, QUERY()) FROM myIndex WHERE MATCH('my.query')
```

इस उदाहरण में, `QUERY()` अभिव्यक्ति वर्तमान पूर्ण-पाठ क्वेरी लौटाती है। `SNIPPET()` गैर-इंडेक्स किए गए टेक्स्ट को भी उजागर कर सकता है:

```sql
mysql  SELECT id, SNIPPET('text to highlight', 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

इसके अतिरिक्त, इसे एक यूज़र-निर्धारित फ़ंक्शन (UDF) का उपयोग करके अन्य स्रोतों से लाए गए टेक्स्ट को उजागर करने के लिए भी उपयोग किया जा सकता है:

```sql
SELECT id, SNIPPET(myUdf(id), 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

इस संदर्भ में, `myUdf()` एक यूज़र-निर्धारित फ़ंक्शन (UDF) है जो एक बाहरी संग्रहण स्रोत से उसके आईडी द्वारा एक दस्तावेज़ प्राप्त करता है। `SNIPPET()` फ़ंक्शन को "पोस्ट लिमिट" फ़ंक्शन माना जाता है, जिसका अर्थ है कि स्निप्पेट की गणना तब तक विलंबित होती है जब तक पूरा अंतिम परिणाम सेट तैयार न हो जाए, और यहां तक कि `LIMIT` धारा लागू होने के बाद भी। उदाहरण के लिए, यदि `LIMIT 20,10` धारा का उपयोग किया जाता है, तो `SNIPPET()` को 10 बार से अधिक नहीं बुलाया जाएगा।

यह ध्यान रखना महत्वपूर्ण है कि `SNIPPET()` फ़ील्ड-आधारित सीमाओं का समर्थन नहीं करता है। इस कार्यक्षमता के लिए, [HIGHLIGHT()](../Searching/Highlighting.md#Highlighting-via-SQL) का उपयोग करें।

### SUBSTRING_INDEX()

<!-- example substring_index -->
`SUBSTRING_INDEX(string, delimiter, number)` मूल स्ट्रिंग का एक उपस्ट्रिंग लौटाता है, जो एक निर्दिष्ट संख्या के आधार पर डेलिमिटर की घटनाओं के आधार पर:

   *   string - मूल स्ट्रिंग, जो एक स्थिर स्ट्रिंग या एक स्ट्रिंग/JSON विशेषता से हो सकती है।
   *   delimiter - खोजने के लिए डेलिमिटर।
   *   number - डेलिमिटर के लिए खोजने की次数। यह एक सकारात्मक या नकारात्मक संख्या हो सकती है। यदि यह एक सकारात्मक संख्या है, तो फ़ंक्शन डेलिमिटर के बाईं ओर सब कुछ लौटाएगा। यदि यह एक नकारात्मक संख्या है, तो फ़ंक्शन डेलिमिटर के दाईं ओर सब कुछ लौटाएगा।

`SUBSTRING_INDEX()` डिफ़ॉल्ट रूप से एक स्ट्रिंग लौटाता है, लेकिन इसे आवश्यकता पड़ने पर अन्य प्रकार (जैसे पूर्णांक या फ्लोट) में भी बदला जा सकता है। संख्या मानों को विशिष्ट फ़ंक्शंस (जैसे `BIGINT()`, `DOUBLE()`, आदि) का उपयोग करके परिवर्तित किया जा सकता है।

<!-- request SQL -->
```sql
SELECT SUBSTRING_INDEX('www.w3schools.com', '.', 2) FROM test;
SELECT SUBSTRING_INDEX(j.coord, ' ', 1) FROM test;
SELECT          SUBSTRING_INDEX('1.2 3.4', ' ',  1);  /* '1.2' */
SELECT          SUBSTRING_INDEX('1.2 3.4', ' ', -1);  /* '3.4' */
SELECT sint (   SUBSTRING_INDEX('1.2 3.4', ' ',  1)); /* 1 */
SELECT sint (   SUBSTRING_INDEX('1.2 3.4', ' ', -1)); /* 3 */
SELECT double ( SUBSTRING_INDEX('1.2 3.4', ' ',  1)); /* 1.200000 */
SELECT double ( SUBSTRING_INDEX('1.2 3.4', ' ', -1)); /* 3.400000 */
```

<!-- end -->

### UPPER() और LOWER()

`UPPER(string)` तर्क को अपर केस में परिवर्तित करता है, `LOWER(string)` तर्क को लोअर केस में परिवर्तित करता है।

परिणाम को नंबर में बढ़ाया जा सकता है, लेकिन केवल यदि स्ट्रिंग तर्क संख्या में परिवर्तनीय है। संख्या मानों को आक्रामक फ़ंक्शंस (`BITINT`, `DOUBLE`, आदि) के साथ बढ़ाया जा सकता है।

```sql
SELECT upper('www.w3schools.com', '.', 2); /* WWW.W3SCHOOLS.COM  */
SELECT double (upper ('1.2e3')); /* 1200.000000 */
SELECT integer (lower ('12345')); /* 12345 */
```
<!-- proofread -->
