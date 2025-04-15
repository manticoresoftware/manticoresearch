# एक तालिका को खाली करना

तालिका को `TRUNCATE TABLE` SQL स्टेटमेंट या `truncate()` PHP क्लाइंट फ़ंक्शन के साथ खाली किया जा सकता है।

यहां SQL स्टेटमेंट का सिंटैक्स है:

```sql
TRUNCATE TABLE table_name [WITH RECONFIGURE]
```

<!-- example truncate -->

जब यह स्टेटमेंट निष्पादित किया जाता है, तो यह RT तालिका को पूरी तरह से साफ कर देता है। यह इन-मेमोरी डेटा को नष्ट कर देता है, सभी तालिका डेटा फ़ाइलों को अनलिंक कर देता है, और संबंधित बाइनरी लॉग्स को मुक्त कर देता है।

एक तालिका को `DELETE FROM index WHERE id>0` के साथ भी खाली किया जा सकता है, लेकिन इसकी सिफारिश नहीं की जाती क्योंकि यह `TRUNCATE` की तुलना में धीमा है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
TRUNCATE TABLE products;
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```http
POST /cli -d "TRUNCATE TABLE products"
```

<!-- response JSON -->
```http
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$params = [ 'table' => 'products' ];
$response = $client->indices()->truncate($params);
```

<!-- response PHP -->
```php
Array(
    [total] => 0
    [error] => 
    [warning] => 
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('TRUNCATE TABLE products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('TRUNCATE TABLE products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("TRUNCATE TABLE products");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("TRUNCATE TABLE products");
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- end -->

इस कमांड का एक संभावित उपयोग [तालिका को संलग्न करने] (Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) से पहले है।

<!-- example truncate with RECONFIGURE -->

जब `RECONFIGURE` विकल्प का उपयोग किया जाता है, तो नई टोकनाइजेशन, morphological, और अन्य पाठ प्रसंस्करण सेटिंग्स जो कॉन्फ़िग में निर्दिष्ट की गई हैं, तालिका के साफ़ होने के बाद प्रभावी हो जाती हैं। यदि कॉन्फ़िग में [schema declaration] (Creating_a_table/Data_types.md) तालिका स्कीमा से भिन्न है, तो तालिका के क्लियर होने के बाद कॉन्फ़िग से नया स्कीमा लागू किया गया है।

इस विकल्प के साथ, एक तालिका को साफ़ करना और पुनः कॉन्फ़िगर करना एक परमाणु संचालन बन जाता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
TRUNCATE TABLE products with reconfigure;
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### JSON:

<!-- request HTTP -->

```http
POST /cli -d "TRUNCATE TABLE products with reconfigure"
```

<!-- response HTTP -->
```http
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$params = [ 'table' => 'products', 'with' => 'reconfigure' ];
$response = $client->indices()->truncate($params);
```

<!-- response PHP -->
```php
Array(
    [total] => 0
    [error] => 
    [warning] => 
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('TRUNCATE TABLE products WITH RECONFIGURE')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('TRUNCATE TABLE products WITH RECONFIGURE');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("TRUNCATE TABLE products WITH RECONFIGURE");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("TRUNCATE TABLE products WITH RECONFIGURE");
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```
<!-- end -->
<!-- proofread -->
