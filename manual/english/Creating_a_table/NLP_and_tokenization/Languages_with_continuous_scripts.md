# Chinese, Japanese and Korean (CJK) and Thai languages

Manticore provides built-in support for indexing languages with [continuous scripts](https://en.wikipedia.org/wiki/Scriptio_continua) (i.e., languages that do not use spaces or other marks between words or sentences). This allows you to process texts in these languages in two different ways:

<!-- example cont 1 -->
1. Precise segmentation using the [ICU](https://icu.unicode.org/) library. Currently, only Chinese is supported.


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
2. Precise segmentation using the [Jieba](https://github.com/fxsjy/jieba) library. Like ICU, it currently supports only Chinese.

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
3. Basic support using the N-gram options [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) and [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)
For each language using a continuous script, there are separate character set tables (`chinese`, `korean`, `japanese`, `thai`) that can be used. Alternatively, you can use the common `cont` character set table to support all CJK and Thai languages at once, or the `cjk` charset to include all CJK languages only.


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
Additionally, there is built-in support for Chinese [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)  with the alias `zh`.


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
