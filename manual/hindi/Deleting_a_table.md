# एक तालिका को हटाना

<!-- example drop -->

एक तालिका को हटाने की प्रक्रिया आंतरिक रूप से 2 चरणों में की जाती है:
1. तालिका को साफ किया जाता है (जैसे [TRUNCATE](Emptying_a_table.md))
2. सभी तालिका फ़ाइलों को तालिका फ़ोल्डर से हटा दिया जाता है। सभी बाहरी तालिका फ़ाइलें जो तालिका द्वारा उपयोग की गई थीं (जैसे शब्द रूप, एक्सटेंशन या स्टॉपवर्ड) भी हटा दी जाती हैं। ध्यान दें कि ये बाहरी फ़ाइलें `CREATE TABLE` के उपयोग के समय तालिका फ़ोल्डर में कॉपी की जाती हैं, इसलिए `CREATE TABLE` में बताई गई मूल फ़ाइलें हटाई नहीं जाएंगी।

एक तालिका को हटाना संभव है केवल तभी जब सर्वर RT मोड में चल रहा हो। RT तालिकाओं, PQ तालिकाओं और वितरित तालिकाओं को हटाना संभव है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
DROP TABLE products;
```
<!-- response -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "DROP TABLE products"
```

<!-- response JSON -->
```JSON
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

$response = $client->indices()->drop($params);
```

<!-- response PHP -->
```php
Array
(
    [total] => 0
    [error] =>
    [warning] =>
)

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('DROP TABLE products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('DROP TABLE products')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('DROP TABLE products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
sqlresult = utilsApi.sql("DROP TABLE products", true);
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
sqlresult = utilsApi.Sql("DROP TABLE products", true);
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
let sqlresult = utils_api.sql("DROP TABLE products", Some(true)).await;
```

<!-- response Rust -->
```rust
{total=0, error="", warning=""}
```

<!-- end -->

यहां SQL में `DROP TABLE` कथन की क्रिया की संरचना है:

```sql
DROP TABLE [IF EXISTS] table_name
```

<!-- example drop-if-exists -->

SQL के माध्यम से तालिका को हटाते समय, `IF EXISTS` जोड़ना तालिका को केवल तब हटाने के लिए उपयोग किया जा सकता है जब यह मौजूद हो। यदि आप `IF EXISTS` विकल्प के साथ एक गैर-मौजूद तालिका को हटाने की कोशिश करते हैं, तो कुछ नहीं होता।

PHP के माध्यम से तालिका को हटाते समय, आप एक वैकल्पिक `silent` पैरामीटर जोड़ सकते हैं जो `IF EXISTS` की तरह कार्य करता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
DROP TABLE IF EXISTS products;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```http
POST /cli -d "DROP TABLE IF EXISTS products"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$params =
[
  'table' => 'products',
  'body' => ['silent' => true]
];

$client->indices()->drop($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('DROP TABLE IF EXISTS products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('DROP TABLE IF EXISTS products')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('DROP TABLE IF EXISTS products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
sqlresult = utilsApi.sql("DROP TABLE IF EXISTS products", true);
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
sqlresult = utilsApi.Sql("DROP TABLE IF EXISTS products", true);
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
let sqlresult = utils_api.sql("DROP TABLE IF EXISTS products", Some(true)).await;
```

<!-- response Rust -->
```rust
{total=0, error="", warning=""}
```

<!-- end -->
<!-- proofread -->
# एक तालिका हटाना

<!-- example drop -->

एक तालिका हटाने के लिए आंतरिक रूप से 2 चरणों में कार्य किया जाता है:
1. तालिका को साफ किया जाता है (जो [TRUNCATE](Emptying_a_table.md) के समान है)
2. तालिका फोल्डर से सभी तालिका फ़ाइलें हटा दी जाती हैं। सभी बाहरी तालिका फ़ाइलें जो तालिका द्वारा उपयोग की गई थीं (जैसे wordforms, extensions या stopwords) भी हटा दी जाती हैं। ध्यान दें कि ये बाहरी फ़ाइलें `CREATE TABLE` का उपयोग करते समय तालिका फोल्डर में कॉपी की जाती हैं, इसलिए `CREATE TABLE` में निर्दिष्ट मूल फ़ाइलें नहीं हटाई जाएंगी।

एक तालिका हटाना केवल तभी संभव है जब सर्वर RT मोड में चल रहा हो। RT तालिकाएँ, PQ तालिकाएँ और वितरित तालिकाएँ हटाई जा सकती हैं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
DROP TABLE products;
```
<!-- response -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "DROP TABLE products"
```

<!-- response JSON -->
```JSON
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

$response = $client->indices()->drop($params);
```

<!-- response PHP -->
```php
Array
(
    [total] => 0
    [error] =>
    [warning] =>
)

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('DROP TABLE products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('DROP TABLE products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
sqlresult = utilsApi.sql("DROP TABLE products");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
sqlresult = utilsApi.Sql("DROP TABLE products");
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- end -->

यहाँ SQL में `DROP TABLE` बयान की वाक्यविन्यास है:

```sql
DROP TABLE [IF EXISTS] table_name
```

<!-- example drop-if-exists -->

SQL के माध्यम से एक तालिका हटाते समय, तालिका को केवल हटाने के लिए `IF EXISTS` जोड़ना उपयोग किया जा सकता है यदि यह मौजूद है। यदि आप `IF EXISTS` विकल्प के साथ एक गैर-मौजूद तालिका को हटाने का प्रयास करते हैं, तो कुछ भी नहीं होता है।

PHP के माध्यम से एक तालिका हटाते समय, आप एक वैकल्पिक `silent` पैरामीटर जोड़ सकते हैं जो `IF EXISTS` के समान कार्य करता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
DROP TABLE IF EXISTS products;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```http
POST /cli -d "DROP TABLE IF EXISTS products"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$params =
[
  'table' => 'products',
  'body' => ['silent' => true]
];

$client->indices()->drop($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('DROP TABLE IF EXISTS products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('DROP TABLE IF EXISTS products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
sqlresult = utilsApi.sql("DROP TABLE IF EXISTS products");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
sqlresult = utilsApi.Sql("DROP TABLE IF EXISTS products");
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- end -->
<!-- proofread -->
