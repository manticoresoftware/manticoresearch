# तालिकाएँ सूचीबद्ध करना

Manticore Search में तालिकाओं के लिए एकल स्तर की हायरार्की है।

अन्य DBMS के विपरीत, Manticore में तालिकाओं को डेटाबेस में समूहबद्ध करने का कोई विचार नहीं है। हालाँकि, SQL बोलियों के साथ इंटरऑपरेबिलिटी के लिए, Manticore `SHOW DATABASES` बयानों को SQL बोली के साथ इंटरऑपरेबिलिटी के लिए स्वीकार करता है, लेकिन यह बयाँ कोई परिणाम नहीं लौटाता है।

<!-- example listing -->
## SHOW TABLES

सामान्य सिंटैक्स:

```sql
SHOW TABLES [ LIKE pattern ]
```

`SHOW TABLES` बयान सभी वर्तमान सक्रिय तालिकाओं को उनके प्रकारों के साथ सूचीबद्ध करता है। मौजूदा तालिका प्रकार हैं `local`, `distributed`, `rt`, `percolate` और `template`।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES;
```

<!-- response SQL -->

```sql
+----------+-------------+
| Index    | Type        |
+----------+-------------+
| dist     | distributed |
| plain    | local       |
| pq       | percolate   |
| rt       | rt          |
| template | template    |
+----------+-------------+
5 rows in set (0.00 sec)
```

<!-- request PHP -->

```php
$client->nodes()->table();
```

<!-- response PHP -->

```php
Array
(
    [dist1] => distributed
    [rt] => rt
    [products] => rt
)

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW TABLES')
```

<!-- response Python -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'dist1', u'Type': u'distributed'},
           {u'Index': u'rt', u'Type': u'rt'},
           {u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW TABLES')
```

<!-- response Python-asyncio -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'dist1', u'Type': u'distributed'},
           {u'Index': u'rt', u'Type': u'rt'},
           {u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLES');
```

<!-- response javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES", true)
```

<!-- response Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES", true)
```

<!-- response C# -->
```C#
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("SHOW TABLES", Some(true)).await
```

<!-- response Rust -->
```Rust
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```

<!-- end -->

<!-- example Example_2 -->
वैकल्पिक LIKE क्लॉज़ नाम के आधार पर तालिकाओं को फ़िल्टर करने के लिए समर्थित है।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES LIKE 'pro%';
```

<!-- response SQL -->

```sql
+----------+-------------+
| Index    | Type        |
+----------+-------------+
| products | distributed |
+----------+-------------+
1 row in set (0.00 sec)
```

<!-- request PHP -->

```php
$client->nodes()->table(['body'=>['pattern'=>'pro%']]);
```

<!-- response PHP -->

```php
Array
(
    [products] => distributed
)

```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW TABLES LIKE 'pro%'');
```

<!-- response Python -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW TABLES LIKE 'pro%'');
```

<!-- response Python-asyncio -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
utilsApi.sql('SHOW TABLES LIKE 'pro%'')
```

<!-- response javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```


<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES LIKE 'pro%'", true)
```

<!-- response Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES LIKE 'pro%'", true)
```

<!-- response C# -->
```clike
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```

<!-- intro -->

##### Rust:

<!-- request Rust -->

```rust

utils_api.sql("SHOW TABLES LIKE 'pro%'", Some(true)).await

```

<!-- response Rust -->

```rust

{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}

```

<!-- end -->

## DESCRIBE

```sql

{DESC | DESCRIBE} table_name [ LIKE pattern ]

```

`DESCRIBE` बयान तालिका के कॉलम और उनके संबंधित प्रकारों को सूचीबद्ध करता है। कॉलम दस्तावेज़ आईडी, पूर्ण-पाठ फ़ील्ड और विशेषताओं होते हैं। क्रम `INSERT` और `REPLACE` बयानों द्वारा अपेक्षित फ़ील्ड और विशेषताओं के क्रम से मेल खाता है। कॉलम प्रकार में `field`, `integer`, `timestamp`, `ordinal`, `bool`, `float`, `bigint`, `string`, और `mva` शामिल हैं। आईडी कॉलम को `bigint` के रूप में टाइप किया जाएगा। उदाहरण:

```sql

mysql> DESC rt;

+---------+---------+

| Field   | Type    |

+---------+---------+

| id      | bigint  |

| title   | field   |

| content | field   |

| gid     | integer |

+---------+---------+

4 rows in set (0.00 sec)

```

एक वैकल्पिक LIKE खंड समर्थित है। इसके वाक्यांश विवरण के लिए 
[SHOW META](Node_info_and_management/SHOW_META.md) को संदर्भित करें।

### SELECT FROM name.@table

<!-- example name_table -->

आप `select * from <table_name>.@table` क्वेरी को निष्पादित करके तालिका की योजना भी देख सकते हैं। इस विधि का लाभ यह है कि आप फ़िल्टरिंग के लिए `WHERE` खंड का उपयोग कर सकते हैं:

<!-- request SQL -->

```sql

select * from tbl.@table where type='text';

```

<!-- response SQL -->

```sql

+------+-------+------+----------------+

| id   | field | type | properties     |

+------+-------+------+----------------+

|    2 | title | text | indexed stored |

+------+-------+------+----------------+

1 row in set (0.00 sec)

```

<!-- end -->

<!-- example name_table2 -->

आप `<your_table_name>.@table` पर और भी कई कार्य कर सकते हैं, इसे पूर्ण नियमित मैन्टिकोर तालिका मानकर जिसमें कॉलम में पूर्णांक और स्ट्रिंग विशेषताएँ शामिल हैं।

<!-- request SQL -->

```sql

select field from tbl.@table;

select field, properties from tbl.@table where type in ('text', 'uint');

select * from tbl.@table where properties any ('stored');

```

<!-- end -->

## SHOW CREATE TABLE

<!-- example show_create -->

```sql

SHOW CREATE TABLE table_name

```

निर्दिष्ट तालिका बनाने के लिए उपयोग किया जाने वाला `CREATE TABLE` बयान प्रिंट करता है।

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql

SHOW CREATE TABLE tblG

```

<!-- response SQL -->

```sql

       Table: tbl

Create Table: CREATE TABLE tbl (

f text indexed stored

) charset_table='non_cont,cont' morphology='icu_chinese'

# तालिकाएं सूचीबद्ध करना


Manticore Search तालिकाओं के लिए एक एकल स्तर की पदानुक्रम संरचना प्रदान करता है।


अन्य DBMS की तरह, Manticore में तालिकाओं को डेटाबेस में समूहित करने की अवधारणा नहीं है। हालाँकि, SQL बोलीभाषाओं के साथ इंटरऑपरेबिलिटी के लिए, Manticore `SHOW DATABASES` कथनों को स्वीकार करता है, लेकिन यह कथन कोई परिणाम वापस नहीं करता।


<!-- example listing -->
## तालिकाएं दिखाएँ


General syntax:


```sql
SHOW TABLES [ LIKE pattern ]

```


`SHOW TABLES` कथन वर्तमान में सक्रिय सभी तालिकाओं को उनके प्रकारों के साथ सूचीबद्ध करता है। मौजूदा तालिका प्रकार `local`, `distributed`, `rt`, `percolate` और `template` हैं।



<!-- intro -->
##### SQL:


<!-- request SQL -->

```sql
SHOW TABLES;

```


<!-- response SQL -->

```sql
+----------+-------------+
| Index    | Type        |
+----------+-------------+
| dist     | distributed |
| plain    | local       |
| pq       | percolate   |
| rt       | rt          |
| template | template    |
+----------+-------------+
5 rows in set (0.00 sec)

```


<!-- request PHP -->

```php
$client->nodes()->table();

```


<!-- response PHP -->

```php
Array
(
    [dist1] => distributed
    [rt] => rt
    [products] => rt
)

```
<!-- intro -->
##### Python:


<!-- request Python -->

```python
utilsApi.sql('SHOW TABLES')

```


<!-- response Python -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'dist1', u'Type': u'distributed'},
           {u'Index': u'rt', u'Type': u'rt'},
           {u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}

```

<!-- intro -->
##### Javascript:


<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLES');

```


<!-- response javascript -->

```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}

```


<!-- intro -->
##### java:


<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES")

```


<!-- response Java -->

```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}

```


<!-- intro -->
##### C#:


<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES")

```


<!-- response C# -->

```C#
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}

```


<!-- end -->


<!-- example Example_2 -->
नाम द्वारा तालिकाओं को फ़िल्टर करने के लिए वैकल्पिक LIKE क्लॉज़ समर्थित है।



<!-- intro -->
##### SQL:


<!-- request SQL -->

```sql
SHOW TABLES LIKE 'pro%';

```


<!-- response SQL -->

```sql
+----------+-------------+
| Index    | Type        |
+----------+-------------+
| products | distributed |
+----------+-------------+
1 row in set (0.00 sec)

```


<!-- request PHP -->

```php
$client->nodes()->table(['body'=>['pattern'=>'pro%']]);

```


<!-- response PHP -->

```php
Array
(
    [products] => distributed
)

```



<!-- intro -->
##### Python:


<!-- request Python -->

```python
res = await utilsApi.sql('SHOW TABLES LIKE \'pro%\'');

```


<!-- response Python -->

```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}

```

<!-- intro -->
##### Javascript:


<!-- request javascript -->

```javascript
utilsApi.sql('SHOW TABLES LIKE \'pro%\'')

```


<!-- response javascript -->

```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}

```



<!-- intro -->
##### java:


<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES LIKE 'pro%'")

```


<!-- response Java -->

```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}

```


<!-- intro -->
##### C#:


<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES LIKE 'pro%'")

```


<!-- response C# -->

```clike
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}

```



<!-- end -->


## विवरण


```sql
{DESC | DESCRIBE} table_name [ LIKE pattern ]

```


`DESCRIBE` कथन तालिका के कॉलम और उनके संबद्ध प्रकारों को सूचीबद्ध करता है। कॉलम दस्तावेज़ आईडी, पूर्ण-पाठ फ़ील्ड और विशेषताएँ हैं। क्रम उसी क्रम से मेल खाता है जिसमें फ़ील्ड और विशेषताओं की अपेक्षा `INSERT` और `REPLACE` कथनों द्वारा की जाती है। कॉलम प्रकारों में `field`, `integer`, `timestamp`, `ordinal`, `bool`, `float`, `bigint`, `string` और `mva` शामिल हैं। आईडी कॉलम का प्रकार `bigint` होगा। Example:


```sql
mysql> DESC rt;
+---------+---------+
| Field   | Type    |
+---------+---------+
| id      | bigint  |
| title   | field   |
| content | field   |
| gid     | integer |
+---------+---------+
4 rows in set (0.00 sec)

```


एक वैकल्पिक LIKE क्लॉज़ समर्थित है। Refer to
[SHOW META](Node_info_and_management/SHOW_META.md) इसके वाक्य रचना विवरण के लिए।


### name.@table से SELECT


<!-- example name_table -->
आप क्वेरी `select * from <table_name>.@table` को निष्पादित करके तालिका योजना भी देख सकते हैं। इस विधि का लाभ यह है कि आप फ़िल्टरिंग के लिए `WHERE` क्लॉज़ का उपयोग कर सकते हैं:


<!-- request SQL -->

```sql
select * from tbl.@table where type='text';

```


<!-- response SQL -->
```sql
+------+-------+------+----------------+
| id   | field | type | properties     |
+------+-------+------+----------------+
|    2 | title | text | indexed stored |
+------+-------+------+----------------+
1 row in set (0.00 sec)
```

