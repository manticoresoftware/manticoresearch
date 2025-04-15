# चीनी, जापानी और कोरियाई (CJK) और थाई भाषाएँ

मैन्टिकोर निरंतर स्क्रिप्टों के लिए इनडेक्सिंग भाषाओं के लिए निर्मित समर्थन प्रदान करता है (अर्थात्, ऐसी भाषाएँ जो शब्दों या वाक्यों के बीच स्थान या अन्य चिह्नों का उपयोग नहीं करती हैं)। यह आपको इन भाषाओं में पाठ्यक्रम को दो अलग-अलग तरीकों से संसाधित करने की अनुमति देता है:

<!-- example cont 1 -->
1. [ICU](https://icu.unicode.org/) पुस्तकालय का उपयोग करके सटीक विभाजन। वर्तमान में, केवल चीनी समर्थित है।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'icu_chinese'
```

<!-- request JSON -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'icu_chinese'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'charset_table' => 'cont',
            'morphology' => 'icu_chinese'
        ]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'cont\' morphology = \'icu_chinese\'')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'cont\' morphology = \'icu_chinese\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'icu_chinese'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'icu_chinese'");
```

<!-- request CONFIG -->

```ini
table products {
  charset_table = cont
  morphology = icu_chinese

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example cont 2 -->
2. [Jieba](https://github.com/fxsjy/jieba) पुस्तकालय का उपयोग करके सटीक विभाजन। ICU की तरह, वर्तमान में यह केवल चीनी का समर्थन करता है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'jieba_chinese'
```

<!-- request JSON -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'jieba_chinese'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'charset_table' => 'cont',
            'morphology' => 'jieba_chinese'
        ]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'cont\' morphology = \'jieba_chinese\'')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'cont\' morphology = \'jieba_chinese\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'jieba_chinese'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'jieba_chinese'");
```

<!-- request CONFIG -->

```ini
table products {
  charset_table = cont
  morphology = jieba_chinese

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example cont 3 -->
3. N-ग्राम विकल्पों [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) और [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) का मूल समर्थन
निरंतर स्क्रिप्ट का उपयोग करने वाली प्रत्येक भाषा के लिए, विभिन्न वर्ण सेट तालिकाएँ (`chinese`, `korean`, `japanese`, `thai`) उपलब्ध हैं। वैकल्पित रूप से, आप सभी CJK और थाई भाषाओं का समर्थन करने के लिए सामान्य `cont` वर्ण सेट तालिका का उपयोग कर सकते हैं, या केवल सभी CJK भाषाएँ शामिल करने के लिए `cjk` charset का उपयोग कर सकते हैं।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'

/* Or, alternatively */
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cjk,thai'
```

<!-- request JSON -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'"

/* Or, alternatively */
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cjk,thai'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'charset_table' => 'non_cont',
             'ngram_len' => '1',
             'ngram_chars' => 'cont'
        ]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'");
```

<!-- request CONFIG -->

```ini
table products {
  charset_table = non_cont
  ngram_len = 1
  ngram_chars = cont

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example cont 3_2 -->
इसके अलावा, चीनी [स्टॉपवर्ड्स](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) के लिए अंतर्निहित समर्थन है जिसकी उपमा `zh` है।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'chinese' morphology = 'icu_chinese' stopwords = 'zh'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'chinese' morphology = 'icu_chinese' stopwords = 'zh'"
```
<!-- intro -->
##### PHP:
<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'charset_table' => 'chinese',
            'morphology' => 'icu_chinese',
            'stopwords' => 'zh'
        ]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'chinese\' morphology = \'icu_chinese\' stopwords = \'zh\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'chinese\' morphology = \'icu_chinese\' stopwords = \'zh\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'chinese' morphology = 'icu_chinese' stopwords = 'zh'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'chinese' morphology = 'icu_chinese' stopwords = 'zh'");
```

<!-- request CONFIG -->

```ini
table products {
  charset_table = chinese
  morphology = icu_chinese
  stopwords = zh

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->
