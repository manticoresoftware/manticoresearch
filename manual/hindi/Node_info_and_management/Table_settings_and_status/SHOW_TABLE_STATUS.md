# SHOW TABLE STATUS

<!-- उदाहरण SHOW TABLE STATUS -->

`SHOW TABLE STATUS` एक SQL वाक्यांश है जो विभिन्न प्रति-तालिका आंकड़े दिखाता है।

वाक्यविन्यास है:

```sql
SHOW TABLE table_name STATUS
```

अनुक्रमणिका प्रकार के आधार पर, प्रदर्शित आंकड़े विभिन्न पंक्तियों के सेट को शामिल करते हैं:

* **template**: `index_type`.
* **distributed**: `index_type`, `query_time_1min`, `query_time_5min`, `query_time_15min`, `query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **percolate**: `index_type`, `stored_queries`, `ram_bytes`, `disk_bytes`, `max_stack_need`, `average_stack_base`, `
  desired_thread_stack`, `tid`, `tid_saved`, `query_time_1min`, `query_time_5min`, `query_time_15min`, `query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **plain**: `index_type`, `indexed_documents`, `indexed_bytes`, हो सकता है `field_tokens_*` और `total_tokens`, `ram_bytes`, `disk_bytes`, `disk_mapped`, `disk_mapped_cached`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists`, `disk_mapped_cached_hitlists`, `killed_documents`, `killed_rate`, `query_time_1min`, `query_time_5min`, `query_time_15min`, `query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **rt**: `index_type`, `indexed_documents`, `indexed_bytes`, हो सकता है `field_tokens_*` और `total_tokens`, `ram_bytes`, `disk_bytes`, `disk_mapped`, `disk_mapped_cached`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists`, `disk_mapped_cached_hitlists`, `killed_documents`, `killed_rate`, `ram_chunk`, `ram_chunk_segments_count`, `disk_chunks`, `mem_limit`, `mem_limit_rate`, `ram_bytes_retired`, `optimizing`, `locked`, `tid`, `tid_saved`, `query_time_1min`, `query_time_5min`, `query_time_15min`, `query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.

यहाँ इन मानों का अर्थ है:

* `index_type`: वर्तमान में `disk`, `rt`, `percolate`, `template`, और `distributed` में से एक।
* `indexed_documents`: अनुक्रमित दस्तावेजों की संख्या।
* `indexed_bytes`: अनुक्रमित पाठ का कुल आकार। ध्यान दें, यह मान सख्त नहीं है, क्योंकि पूर्ण-पाठ अनुक्रमणिका में सहेजे गए पाठ को सख्ती से पुनर्प्राप्त करना असंभव है।
* `stored_queries`: तालिका में संग्रहित पर्कोलेट प्रश्नों की संख्या।
* `field_tokens_XXX`: वैकल्पिक, संपूर्ण तालिका में प्रति-क्षेत्र लंबाई का योग (टोकन में) (इंटरनली `BM25A` और `BM25F` रैंकिंग फ़ंक्शन के लिए उपयोग किया जाता है)। केवल उन तालिकाओं के लिए उपलब्ध है जो `index_field_lengths=1` के साथ निर्मित हैं।
* `total_tokens`: वैकल्पिक, सभी `field_tokens_XXX` का कुल योग।
* `ram_bytes`: तालिका द्वारा कब्जा की गई कुल RAM।
* `disk_bytes`: तालिका द्वारा कब्जा की गई कुल डिस्क स्पेस।
* `disk_mapped`: फ़ाइल मैपिंग का कुल आकार।
* `disk_mapped_cached`: RAM में वास्तव में कैश की गई फ़ाइल मैपिंग का कुल आकार।
* `disk_mapped_doclists` और `disk_mapped_cached_doclists`: दस्तावेज़ सूचियों से संबंधित कुल और कैश किए गए मैपिंग का भाग।
* `disk_mapped_hitlists` और `disk_mapped_cached_hitlists`: हिट सूचियों से संबंधित कुल और कैश किए गए मैपिंग का भाग। दस्तावेज़ सूचियाँ और हिट सूचियों के मान अलग-अलग दिखाए जाते हैं क्योंकि वे आमतौर पर बड़े होते हैं (उदाहरण के लिए, पूरे तालिका के आकार का लगभग 90%)।
* `killed_documents` और `killed_rate`: पहला हटाए गए दस्तावेजों की संख्या और हटाए गए/अनुक्रमित की दर को दर्शाता है। तकनीकी रूप से, दस्तावेज़ को हटाना खोज परिणाम में उसे दबाना होता है, लेकिन यह अभी भी तालिका में भौतिक रूप से मौजूद है और केवल तालिका को विलय/अनुकूलित करने के बाद ही हटाया जाएगा।
* `ram_chunk`: वास्तविक समय या पर्कोलेट तालिका के RAM चंक का आकार।
* `ram_chunk_segments_count`: RAM चंक आंतरिक रूप से खंडों से बना होता है, सामान्यत: 32 से अधिक नहीं। यह पंक्ति वर्तमान गिनती को दिखाती है।
* `disk_chunks`: वास्तविक समय तालिका में डिस्क चंकों की संख्या।
* `mem_limit`: तालिका के लिए `rt_mem_limit` का वास्तविक मान।
* `mem_limit_rate`: वह दर जिस पर RAM चंक डिस्क चंक के रूप में फ्लश किया जाएगा, उदाहरण के लिए, यदि `rt_mem_limit` 128M है और दर 50% है, तो एक नया डिस्क चंक तब सहेजा जाएगा जब RAM चंक 64M से अधिक हो।
* `ram_bytes_retired`: RAM चंकों में कचरे का आकार दर्शाता है (जैसे, हटाए गए या प्रतिस्थापित दस्तावेज जिन्हें अभी तक स्थायी रूप से हटाया नहीं गया है)।
* `optimizing`: 0 से अधिक मान दर्शाता है कि तालिका इस समय अनुकूलन कर रही है (यानी यह अभी कुछ डिस्क चंकों को जोड़ रही है)।
* `locked`: 0 से अधिक मान दर्शाता है कि तालिका वर्तमान में [FREEZE](../../Securing_and_compacting_a_table/Freezing_a_table.md#Freezing-a-table) द्वारा लॉक की गई है। संख्या दर्शाती है कि तालिका कितनी बार फ्रीज़ की गई है। उदाहरण के लिए, एक तालिका को `manticore-backup` द्वारा फ्रीज़ किया जा सकता है और फिर पुनरावृत्ति द्वारा फिर से फ्रीज़ किया जा सकता है। इसे तब तक पूरी तरह से अनफ्रीज़ नहीं किया जाना चाहिए जब तक कि कोई अन्य प्रक्रिया इसे फ्रीज़ करने की आवश्यकता न रखे।
* `max_stack_need`: स्टोर किए गए पर्कोलेट प्रश्नों से सबसे जटिल की गणना के लिए हमें जितनी स्टैक जगह की आवश्यकता है। यह एक गतिशील मान है, जो निर्माण विवरणों जैसे कि कंपाइलर, अनुकूलन, हार्डवेयर आदि पर निर्भर करता है।
* `average_stack_base`: पर्कोलेट प्रश्न की गणना शुरू करने पर सामान्यत: उपयोग की जाने वाली स्टैक जगह।
* `desired_thread_stack`: ऊपर दिए गए मानों का योग, 128 बाइट के दायरे तक गोल किया गया। यदि यह मान `thread_stack` से अधिक है, तो आप इस तालिका पर `call pq` निष्पादित नहीं कर सकते, क्योंकि कुछ स्टोर किए गए प्रश्न विफल होंगे। अधिकतर `thread_stack` मान 1M है (जो कि 1048576 है); अन्य मानों को कॉन्फ़िगर किया जाना चाहिए।
* `tid` और `tid_saved`: तालिका को सुरक्षित करने की स्थिति व्यक्त करते हैं। `tid` प्रत्येक परिवर्तन (लेन-देन) के साथ बढ़ता है। `tid_saved` RAM मात्रा में सुरक्षित स्थिति का अधिकतम `tid` दिखाता है जो `<table>.ram` फाइल में है। जब संख्या भिन्न होती है, तो कुछ परिवर्तन केवल RAM में होते हैं और बिनलॉग द्वारा समर्थित होते हैं (यदि सक्षम हो)। `FLUSH TABLE` करने या नियमित रूप से फ्लशिंग शेड्यूल करने से ये परिवर्तन सुरक्षित होते हैं। फ्लशिंग के बाद, बिनलॉग को साफ किया जाता है, और `tid_saved` नए वास्तविक अवस्था का प्रतिनिधित्व करता है।
* `query_time_*`, `exact_query_time_*`: पिछले 1 मिनट, 5 मिनट, 15 मिनट, और सर्वर स्टार्ट से कुल के लिए क्वेरी निष्पादन समय सांख्यिकी; डेटा JSON ऑब्जेक्ट के रूप में संकुचित है, जिसमें क्वेरी की संख्या और न्यूनतम, अधिकतम, औसत, 95, और 99 प्रतिशत मान शामिल हैं।
* `found_rows_*`: क्वेरियों द्वारा पाए गए पंक्तियों की सांख्यिकी; पिछले 1 मिनट, 5 मिनट, 15 मिनट, और सर्वर स्टार्ट से कुल के लिए प्रदान किया गया; डेटा JSON ऑब्जेक्ट के रूप में संकुचित है, जिसमें क्वेरी की संख्या और न्यूनतम, अधिकतम, औसत, 95, और 99 प्रतिशत मान शामिल हैं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
mysql> SHOW TABLE statistic STATUS;
```

<!-- response SQL -->

```sql
+-----------------------------+--------------------------------------------------------------------------+
| Variable_name               | Value                                                                    |
+-----------------------------+--------------------------------------------------------------------------+
| index_type                  | rt                                                                       |
| indexed_documents           | 146000                                                                   |
| indexed_bytes               | 149504000                                                                |
| ram_bytes                   | 87674788                                                                 |
| disk_bytes                  | 1762811                                                                  |
| disk_mapped                 | 794147                                                                   |
| disk_mapped_cached          | 802816                                                                   |
| disk_mapped_doclists        | 0                                                                        |
| disk_mapped_cached_doclists | 0                                                                        |
| disk_mapped_hitlists        | 0                                                                        |
| disk_mapped_cached_hitlists | 0                                                                        |
| killed_documents            | 0                                                                        |
| killed_rate                 | 0.00%                                                                    |
| ram_chunk                   | 86865484                                                                 |
| ram_chunk_segments_count    | 24                                                                       |
| disk_chunks                 | 1                                                                        |
| mem_limit                   | 134217728                                                                |
| mem_limit_rate              | 95.00%                                                                   |
| ram_bytes_retired           | 0                                                                        |
| optimizing                  | 1                                                                        |
| locked                      | 0                                                                        |
| tid                         | 0                                                                        |
| tid_saved                   | 0                                                                        |
| query_time_1min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_5min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_15min            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_total            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_1min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_5min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_15min            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_total            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
+-----------------------------+--------------------------------------------------------------------------+
29 rows in set (0.00 sec)
```

<!-- intro -->
##### PHP:
<!-- request PHP -->

``` php
$index->status();
```

<!-- response PHP -->

```php
Array(
    [index_type] => rt
    [indexed_documents] => 3
    [indexed_bytes] => 0
    [ram_bytes] => 6678
    [disk_bytes] => 611
    [ram_chunk] => 990
    [ram_chunk_segments_count] => 2
    [mem_limit] => 134217728
    [ram_bytes_retired] => 0
    [optimizing] => 0
    [locked] => 0
    [tid] => 15
    [query_time_1min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_5min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_15min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_total] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [found_rows_1min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_5min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_15min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_total] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}

)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW TABLE statistic STATUS')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
   {u'Key': u'table_type', u'Value': u'rt'}
    {u'Key': u'indexed_documents', u'Value': u'3'}
    {u'Key': u'indexed_bytes', u'Value': u'0'}
    {u'Key': u'ram_bytes', u'Value': u'6678'}
    {u'Key': u'disk_bytes', u'Value': u'611'}
    {u'Key': u'ram_chunk', u'Value': u'990'}
    {u'Key': u'ram_chunk_segments_count', u'Value': u'2'}
    {u'Key': u'mem_limit', u'Value': u'134217728'}
    {u'Key': u'ram_bytes_retired', u'Value': u'0'}
    {u'Key': u'optimizing', u'Value': u'0'}
    {u'Key': u'locked', u'Value': u'0'}
    {u'Key': u'tid', u'Value': u'15'}
    {u'Key': u'query_time_1min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_5min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_15min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_total', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'found_rows_1min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_5min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_15min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_total', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW TABLE statistic STATUS')
```
<!-- response Python-asyncio -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
   {u'Key': u'table_type', u'Value': u'rt'}
    {u'Key': u'indexed_documents', u'Value': u'3'}
    {u'Key': u'indexed_bytes', u'Value': u'0'}
    {u'Key': u'ram_bytes', u'Value': u'6678'}
    {u'Key': u'disk_bytes', u'Value': u'611'}
    {u'Key': u'ram_chunk', u'Value': u'990'}
    {u'Key': u'ram_chunk_segments_count', u'Value': u'2'}
    {u'Key': u'mem_limit', u'Value': u'134217728'}
    {u'Key': u'ram_bytes_retired', u'Value': u'0'}
    {u'Key': u'optimizing', u'Value': u'0'}
    {u'Key': u'locked', u'Value': u'0'}
    {u'Key': u'tid', u'Value': u'15'}
    {u'Key': u'query_time_1min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_5min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_15min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_total', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'found_rows_1min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_5min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_15min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_total', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLE statistic STATUS');
```
<!-- response Javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
   {"Key": "table_type", "Value": "rt"}
    {"Key": "indexed_documents", "Value": "3"}
    {"Key": "indexed_bytes", "Value": "0"}
    {"Key": "ram_bytes", "Value": "6678"}
    {"Key": "disk_bytes", "Value": "611"}
    {"Key": "ram_chunk", "Value": "990"}
    {"Key": "ram_chunk_segments_count", "Value": "2"}
    {"Key": "mem_limit", "Value": "134217728"}
    {"Key": "ram_bytes_retired", "Value": "0"}
    {"Key": "optimizing", "Value": "0"}
    {"Key": "locked", "Value": "0"}
    {"Key": "tid", "Value": "15"}
    {"Key": "query_time_1min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_5min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_15min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_total", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "found_rows_1min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_5min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_15min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}],
 "error": "",
 "total": 0,
 "warning": ""}
```
<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLE statistic STATUS");

```
<!-- response Java -->
```java
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
   { Key=index_type, Value=rt}
    { Key=indexed_documents, Value=3}
    { Key=indexed_bytes, Value=0}
    { Key=ram_bytes, Value=6678}
    { Key=disk_bytes, Value=611}
    { Key=ram_chunk, Value=990}
    { Key=ram_chunk_segments_count, Value=2}
    { Key=mem_limit, Value=134217728}
    { Key=ram_bytes_retired, Value=0}
    { Key=optimizing, Value=0}
    { Key=locked, Value=0}
    { Key=tid, Value=15}
    { Key=query_time_1min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_5min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_15min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_total, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=found_rows_1min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_5min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_15min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
  error= ,
  total=0,
  warning= }
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLE statistic STATUS");

```
<!-- response C# -->
```clike
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
   { Key=index_type, Value=rt}
    { Key=indexed_documents, Value=3}
    { Key=indexed_bytes, Value=0}
    { Key=ram_bytes, Value=6678}
    { Key=disk_bytes, Value=611}
    { Key=ram_chunk, Value=990}
    { Key=ram_chunk_segments_count, Value=2}
    { Key=mem_limit, Value=134217728}
    { Key=ram_bytes_retired, Value=0}
    { Key=optimizing, Value=0}
    { Key=locked, Value=0}
    { Key=tid, Value=15}
    { Key=query_time_1min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_5min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_15min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_total, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=found_rows_1min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_5min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_15min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
  error="" ,
  total=0,
  warning="" }
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("SHOW TABLE statistic STATUS", Some(true)).await;

```
<!-- response Rust -->
```rust
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
   { Key=index_type, Value=rt}
    { Key=indexed_documents, Value=3}
    { Key=indexed_bytes, Value=0}
    { Key=ram_bytes, Value=6678}
    { Key=disk_bytes, Value=611}
    { Key=ram_chunk, Value=990}
    { Key=ram_chunk_segments_count, Value=2}
    { Key=mem_limit, Value=134217728}
    { Key=ram_bytes_retired, Value=0}
    { Key=optimizing, Value=0}
    { Key=locked, Value=0}
    { Key=tid, Value=15}
    { Key=query_time_1min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_5min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_15min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_total, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=found_rows_1min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_5min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_15min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
  error="" ,
  total=0,
  warning="" }
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql('SHOW TABLE statistic STATUS');
```
<!-- response TypeScript -->

```typescript
{

"columns": 

[{

"Key": {"type": "string"}

},
    {
    
"Value": {"type": "string"}
    }],

"data": 

[

{"Key": "table_type", "Value": "rt"}

  {"Key": "indexed_documents", "Value": "3"}

  {"Key": "indexed_bytes", "Value": "0"}

  {"Key": "ram_bytes", "Value": "6678"}

  {"Key": "disk_bytes", "Value": "611"}

  {"Key": "ram_chunk", "Value": "990"}

  {"Key": "ram_chunk_segments_count", "Value": "2"}

  {"Key": "mem_limit", "Value": "134217728"}

  {"Key": "ram_bytes_retired", "Value": "0"}
  {"Key": "अनुकूलित करना", "Value": "0"}

  {"Key": "लॉक किया गया", "Value": "0"}

  {"Key": "tid", "Value": "15"}

  {"Key": "क्वेरी_समय_1मिनट", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}

  {"Key": "क्वेरी_समय_5मिनट", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}

  {"Key": "क्वेरी_समय_15मिनट", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}

  {"Key": "क्वेरी_समय_कुल", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}

  {"Key": "मिले_rows_1मिनट", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}

  {"Key": "मिले_rows_5मिनट", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}

  {"Key": "मिले_rows_15मिनट", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}

  {"Key": "मिले_rows_कुल", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}

],

"error": "",

"total": 0,

"warning": ""
}
```

<!-- intro -->
##### जाओ:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW TABLE statistic STATUS").Execute()
```
<!-- response Go -->

```go
{

"columns": 

[{

"Key": {"type": "string"}

# SHOW TABLE STATUS

<!-- उदाहरण SHOW TABLE STATUS -->

`SHOW TABLE STATUS` एक SQL स्टेटमेंट है जो विभिन्न प्रति-तालिका सांख्यिकी प्रदर्शित करता है।

वाक्य रचना है:

```sql
SHOW TABLE table_name STATUS
```

अनुक्रमणिका के प्रकार के आधार पर, प्रदर्शित सांख्यिकी में विभिन्न पंक्तियों का सेट शामिल है:

* **template**: `index_type`.
* **distributed**: `index_type`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **percolate**: `index_type`, `stored_queries`, `ram_bytes`, `disk_bytes`, `max_stack_need`, `average_stack_base`, `
  desired_thread_stack`, `tid`, `tid_saved`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **plain**: `index_type`, `indexed_documents`, `indexed_bytes`, `field_tokens_*` और `total_tokens` का सेट हो सकता है, `ram_bytes`, `disk_bytes`, `disk_mapped`, `disk_mapped_cached`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists`, `disk_mapped_cached_hitlists`, `killed_documents`, `killed_rate`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **rt**: `index_type`, `indexed_documents`, `indexed_bytes`, `field_tokens_*` और `total_tokens` का सेट हो सकता है, `ram_bytes`, `disk_bytes`, `disk_mapped`, `disk_mapped_cached`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists`, `disk_mapped_cached_hitlists`, `killed_documents`, `killed_rate`, `ram_chunk`, `ram_chunk_segments_count`, `disk_chunks`, `mem_limit`, `mem_limit_rate`, `ram_bytes_retired`, `optimizing`, `locked`, `tid`, `tid_saved`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.

इन मानों का अर्थ है:

* `index_type`: वर्तमान में `disk`, `rt`, `percolate`, `template`, और `distributed` में से एक है।
* `indexed_documents`: अनुक्रमित दस्तावेजों की संख्या।
* `indexed_bytes`: अनुक्रमित पाठ का समग्र आकार। ध्यान दें, यह मान कठोर नहीं है, क्योंकि पूर्ण-पाठ अनुक्रमणिका में संग्रहीत पाठ को सही रूप से पुनर्प्राप्त करना असंभव है।
* `stored_queries`: तालिका में संग्रहीत पर्कोलेट प्रश्नों की संख्या।
* `field_tokens_XXX`: वैकल्पिक, पूरे तालिका में प्रति-क्षेत्र_lengths (tokens में) का कुल माप (आंतरिक रूप से `BM25A` और `BM25F` रैंकिंग कार्यों के लिए उपयोग किया जाता है)। केवल उन तालिकाओं के लिए उपलब्ध है जिनका निर्माण `index_field_lengths=1` के साथ किया गया है।
* `total_tokens`: वैकल्पिक, सभी `field_tokens_XXX` का समग्र योग।
* `ram_bytes`: तालिका द्वारा कब्जा की गई कुल RAM।
* `disk_bytes`: तालिका द्वारा कब्जा किए गए कुल डिस्क स्थान।
* `disk_mapped`: फ़ाइल मानचित्रों का कुल आकार।
* `disk_mapped_cached`: RAM में वास्तव में कैश किए गए फ़ाइल मानचित्रों का कुल आकार।
* `disk_mapped_doclists` और `disk_mapped_cached_doclists`: दस्तावेज़ सूचियों से संबंधित कुल और कैश किए गए मानचित्रों का हिस्सा।
* `disk_mapped_hitlists` और `disk_mapped_cached_hitlists`: हिट सूचियों से संबंधित कुल और कैश किए गए मानचित्रों का हिस्सा। दस्तावेज़ सूचियाँ और हिट सूचियाँ मान अलग से दर्शाए जाते हैं क्योंकि वे आमतौर पर बड़े होते हैं (जैसे, पूरे तालिका के आकार का लगभग 90%)।
* `killed_documents` और `killed_rate`: पहला हटाए गए दस्तावेजों की संख्या को इंगित करता है और हटा/अनुक्रमित की गई दर। तकनीकी रूप से, एक दस्तावेज़ को हटाना इसका खोज परिणाम में दमन करना है, लेकिन यह अभी भी तालिका में भौतिक रूप से मौजूद है और केवल टेबल के विलय/अनुकूलन के बाद ही समाप्त होगा।
* `ram_chunk`: वास्तविक समय या पर्कोलेट तालिका के RAM खंड का आकार।
* `ram_chunk_segments_count`: RAM खंड आंतरिक रूप से खंडों में बना होता है, आमतौर पर 32 से अधिक नहीं। यह पंक्ति वर्तमान संख्या को दर्शाती है।
* `disk_chunks`: वास्तविक समय तालिका में डिस्क खंडों की संख्या।
* `mem_limit`: तालिका के लिए `rt_mem_limit` का वास्तविक मान।
* `mem_limit_rate`: वह दर जिस पर RAM खंड को डिस्क खंड के रूप में फ्लश किया जाएगा, उदाहरण के लिए, यदि `rt_mem_limit` 128M है और दर 50% है, तो जब RAM खंड 64M से अधिक हो जाएगा, एक नया डिस्क खंड सहेजा जाएगा।
* `ram_bytes_retired`: RAM खंडों में कचरे के आकार का प्रतिनिधित्व करता है (जैसे, हटाए गए या प्रतिस्थापित दस्तावेज़ जिन्हें अभी तक स्थायी रूप से हटा नहीं गया है)।
* `optimizing`: 0 से बड़ा मान इंगित करता है कि तालिका वर्तमान में अनुकूलन कर रही है (यानी वर्तमान में कुछ डिस्क खंडों को विलय कर रही है)।
* `locked`: 0 से बड़ा मान इंगित करता है कि तालिका वर्तमान में [FREEZE](../../Securing_and_compacting_a_table/Freezing_a_table.md#Freezing-a-table) द्वारा लॉक की गई है। संख्या दर्शाती है कि तालिका कितनी बार फ्रीज़ की गई है। उदाहरण के लिए, एक तालिका `manticore-backup` द्वारा फ्रीज़ की जा सकती है और फिर पुनरावृत्ति द्वारा फिर से फ्रीज़ किया जा सकता है। इसे पूर्ण रूप से अनफ्रीज़ तब किया जाना चाहिए जब कोई अन्य प्रक्रिया इसे फ्रीज़ करने की आवश्यकता नहीं हो।
* `max_stack_need`: संग्रहित पर्कोलेट प्रश्नों से सबसे जटिल की गणना के लिए हमें जो स्टैक स्थान चाहिए। यह गतिशील मूल्य है, निर्माण के विवरण जैसे संकलक, अनुकूलन, हार्डवेयर, आदि पर निर्भर करता है।
* `average_stack_base`: पर्कोलेट प्रश्न की गणना की शुरुआत में आमतौर पर व्यस्त स्टैक स्थान।
* `desired_thread_stack`: उपरोक्त मानों का योग, 128 बाइट्स के किनारे पर गोल। यदि यह मान `thread_stack` से अधिक है तो आप इस तालिका पर `call pq` निष्पादित नहीं कर सकते, क्योंकि कुछ संग्रहीत प्रश्न विफल हो जाएंगे। डिफ़ॉल्ट `thread_stack` मान 1M है (जो 1048576 है); अन्य मानों को कॉन्फ़िगर किया जाना चाहिए।
* `tid` और `tid_saved`: तालिका की बचत की स्थिति का प्रतिनिधित्व करते हैं। प्रत्येक परिवर्तन (लेन-देन) के साथ `tid` बढ़ता है। `tid_saved` उस स्थिति का अधिकतम `tid` दिखाता है जो `<table>.ram` फ़ाइल में RAM खंड में संग्रहीत है। जब संख्याएँ भिन्न होती हैं, तो कुछ परिवर्तन केवल RAM में होते हैं और बिनलॉग द्वारा भी समर्थित होते हैं (यदि सक्षम किया गया हो)। `FLUSH TABLE` प्रदर्शन करने या नियमित फ्लशिंग को शेड्यूल करने से इन परिवर्तनों को बचाया जाता है। फ्लशिंग के बाद, बिनलॉग साफ हो जाता है, और `tid_saved` नई वास्तविक स्थिति का प्रतिनिधित्व करता है।
* `query_time_*`, `exact_query_time_*`: पिछले 1 मिनट, 5 मिनट, 15 मिनट और सर्वर शुरू होने के बाद कुल के लिए क्वेरी निष्पादन समय सांख्यिकी; डेटा एक JSON ऑब्जेक्ट के रूप में संकुचित होता है, जिसमें क्वेरियों की संख्या और न्यूनतम, अधिकतम, औसत, 95, और 99 प्रतिशत मान शामिल हैं।
* `found_rows_*`: क्वेरियों द्वारा पाए गए पंक्तियों की सांख्यिकी; पिछले 1 मिनट, 5 मिनट, 15 मिनट, और सर्वर शुरू होने के बाद कुल के लिए प्रदान किया गया; डेटा एक JSON ऑब्जेक्ट के रूप में संकुचित होता है, जिसमें क्वेरियों की संख्या और न्यूनतम, अधिकतम, औसत, 95, और 99 प्रतिशत मान शामिल हैं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
mysql> SHOW TABLE statistic STATUS;
```

<!-- response SQL -->

```sql
+-----------------------------+--------------------------------------------------------------------------+
| Variable_name               | Value                                                                    |
+-----------------------------+--------------------------------------------------------------------------+
| index_type                  | rt                                                                       |
| indexed_documents           | 146000                                                                   |
| indexed_bytes               | 149504000                                                                |
| ram_bytes                   | 87674788                                                                 |
| disk_bytes                  | 1762811                                                                  |
| disk_mapped                 | 794147                                                                   |
| disk_mapped_cached          | 802816                                                                   |
| disk_mapped_doclists        | 0                                                                        |
| disk_mapped_cached_doclists | 0                                                                        |
| disk_mapped_hitlists        | 0                                                                        |
| disk_mapped_cached_hitlists | 0                                                                        |
| killed_documents            | 0                                                                        |
| killed_rate                 | 0.00%                                                                    |
| ram_chunk                   | 86865484                                                                 |
| ram_chunk_segments_count    | 24                                                                       |
| disk_chunks                 | 1                                                                        |
| mem_limit                   | 134217728                                                                |
| mem_limit_rate              | 95.00%                                                                   |
| ram_bytes_retired           | 0                                                                        |
| optimizing                  | 1                                                                        |
| locked                      | 0                                                                        |
| tid                         | 0                                                                        |
| tid_saved                   | 0                                                                        |
| query_time_1min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_5min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_15min            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_total            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_1min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_5min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_15min            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_total            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
+-----------------------------+--------------------------------------------------------------------------+
29 rows in set (0.00 sec)
```

<!-- intro -->
##### PHP:
<!-- request PHP -->

``` php
$index->status();
```

<!-- response PHP -->

```php
Array(
    [index_type] => rt
    [indexed_documents] => 3
    [indexed_bytes] => 0
    [ram_bytes] => 6678
    [disk_bytes] => 611
    [ram_chunk] => 990
    [ram_chunk_segments_count] => 2
    [mem_limit] => 134217728
    [ram_bytes_retired] => 0
    [optimizing] => 0
    [locked] => 0
    [tid] => 15
    [query_time_1min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_5min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_15min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_total] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [found_rows_1min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_5min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_15min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_total] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}

)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW TABLE statistic STATUS')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
   {u'Key': u'table_type', u'Value': u'rt'}
    {u'Key': u'indexed_documents', u'Value': u'3'}
    {u'Key': u'indexed_bytes', u'Value': u'0'}
    {u'Key': u'ram_bytes', u'Value': u'6678'}
    {u'Key': u'disk_bytes', u'Value': u'611'}
    {u'Key': u'ram_chunk', u'Value': u'990'}
    {u'Key': u'ram_chunk_segments_count', u'Value': u'2'}
    {u'Key': u'mem_limit', u'Value': u'134217728'}
    {u'Key': u'ram_bytes_retired', u'Value': u'0'}
    {u'Key': u'optimizing', u'Value': u'0'}
    {u'Key': u'locked', u'Value': u'0'}
    {u'Key': u'tid', u'Value': u'15'}
    {u'Key': u'query_time_1min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_5min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_15min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_total', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'found_rows_1min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_5min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_15min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_total', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLE statistic STATUS');
```
<!-- response Javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
   {"Key": "table_type", "Value": "rt"}
    {"Key": "indexed_documents", "Value": "3"}
    {"Key": "indexed_bytes", "Value": "0"}
    {"Key": "ram_bytes", "Value": "6678"}
    {"Key": "disk_bytes", "Value": "611"}
    {"Key": "ram_chunk", "Value": "990"}
    {"Key": "ram_chunk_segments_count", "Value": "2"}
    {"Key": "mem_limit", "Value": "134217728"}
    {"Key": "ram_bytes_retired", "Value": "0"}
    {"Key": "optimizing", "Value": "0"}
    {"Key": "locked", "Value": "0"}
    {"Key": "tid", "Value": "15"}
    {"Key": "query_time_1min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_5min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_15min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_total", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "found_rows_1min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_5min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_15min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}],
 "error": "",
 "total": 0,
 "warning": ""}
```
<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLE statistic STATUS");

```
<!-- response Java -->
```java
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
   { Key=index_type, Value=rt}
    { Key=indexed_documents, Value=3}
    { Key=indexed_bytes, Value=0}
    { Key=ram_bytes, Value=6678}
    { Key=disk_bytes, Value=611}
    { Key=ram_chunk, Value=990}
    { Key=ram_chunk_segments_count, Value=2}
    { Key=mem_limit, Value=134217728}
    { Key=ram_bytes_retired, Value=0}
    { Key=optimizing, Value=0}
    { Key=locked, Value=0}
    { Key=tid, Value=15}
    { Key=query_time_1min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_5min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_15min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_total, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=found_rows_1min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_5min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_15min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
  error= ,
  total=0,
  warning= }
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLE statistic STATUS");

```
<!-- response C# -->
```clike
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
   { Key=index_type, Value=rt}
    { Key=indexed_documents, Value=3}
    { Key=indexed_bytes, Value=0}
    { Key=ram_bytes, Value=6678}
    { Key=disk_bytes, Value=611}
    { Key=ram_chunk, Value=990}
    { Key=ram_chunk_segments_count, Value=2}
    { Key=mem_limit, Value=134217728}
    { Key=ram_bytes_retired, Value=0}
    { Key=optimizing, Value=0}
    { Key=locked, Value=0}
    { Key=tid, Value=15}
    { Key=query_time_1min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_5min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_15min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_total, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=found_rows_1min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_5min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_15min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
  error="" ,
  total=0,
  warning="" }
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql('SHOW TABLE statistic STATUS');
```
<!-- response TypeScript -->

```typescript
{
	"columns": 
	[{
		"Key": {"type": "string"}
	},
    {
    	"Value": {"type": "string"}
    }],
	"data": 
	[
		{"Key": "table_type", "Value": "rt"}
	    {"Key": "indexed_documents", "Value": "3"}
	    {"Key": "indexed_bytes", "Value": "0"}
	    {"Key": "ram_bytes", "Value": "6678"}
	    {"Key": "disk_bytes", "Value": "611"}
	    {"Key": "ram_chunk", "Value": "990"}
	    {"Key": "ram_chunk_segments_count", "Value": "2"}
	    {"Key": "mem_limit", "Value": "134217728"}
	    {"Key": "ram_bytes_retired", "Value": "0"}
	    {"Key": "optimizing", "Value": "0"}
	    {"Key": "locked", "Value": "0"}
	    {"Key": "tid", "Value": "15"}
	    {"Key": "query_time_1min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_5min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_15min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_total", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "found_rows_1min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_5min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_15min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW TABLE statistic STATUS").Execute()
```
<!-- response Go -->

```go
{
	"columns": 
	[{
		"Key": {"type": "string"}
	},
    {
    	"Value": {"type": "string"}
    }],
	"data": 
	[
		{"Key": "table_type", "Value": "rt"}
	    {"Key": "indexed_documents", "Value": "3"}
	    {"Key": "indexed_bytes", "Value": "0"}
	    {"Key": "ram_bytes", "Value": "6678"}
	    {"Key": "disk_bytes", "Value": "611"}
	    {"Key": "ram_chunk", "Value": "990"}
	    {"Key": "ram_chunk_segments_count", "Value": "2"}
	    {"Key": "mem_limit", "Value": "134217728"}
	    {"Key": "ram_bytes_retired", "Value": "0"}
	    {"Key": "optimizing", "Value": "0"}
	    {"Key": "locked", "Value": "0"}
	    {"Key": "tid", "Value": "15"}
	    {"Key": "query_time_1min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_5min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_15min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_total", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "found_rows_1min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_5min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_15min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- end -->
