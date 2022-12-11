# Chinese, Japanese and Korean (CJK) languages

Manticore has built-in support for indexing CJK texts. There are two ways how CJK text can be processed:

<!-- example cjk 1 -->
* Precise segmentation using ICU library (only Chinese is supported for now)


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'cjk' morphology = 'icu_chinese'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'cjk' morphology = 'icu_chinese'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'charset_table' => 'cjk',
            'morphology' => 'icu_chinese'
        ]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'cjk\' morphology = \'icu_chinese\'')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'cjk\' morphology = \'icu_chinese\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'cjk' morphology = 'icu_chinese'");
```
<!-- request CONFIG -->

```ini
table products {
  charset_table = cjk
  morphology = icu_chinese
  
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example cjk 2 -->
* Basic support with N-grams options [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) and [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)
There are separate charset tables(`chinese`, `korean`, `japanese`) that can be used for each CJK-language or, alternatively, common `cjk` charset table can be applied. 


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'non_cjk' ngram_len = '1' ngram_chars = 'cjk'
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'non_cjk' ngram_len = '1' ngram_chars = 'cjk'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'charset_table' => 'non_cjk',
             'ngram_len' => '1',
             'ngram_chars' => 'cjk'
        ]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cjk\' ngram_len = \'1\' ngram_chars = \'cjk\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cjk\' ngram_len = \'1\' ngram_chars = \'cjk\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cjk' ngram_len = '1' ngram_chars = 'cjk'");
```
<!-- request CONFIG -->

```ini
table products {
  charset_table = non_cjk
  ngram_len = 1
  ngram_chars = cjk

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example cjk 3 -->
There's also built-in [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) for Chinese with alias `zh`.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'chinese' morphology = 'icu_chinese' stopwords = 'zh'
```

<!-- request HTTP -->

```http
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
##### javascript:

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
