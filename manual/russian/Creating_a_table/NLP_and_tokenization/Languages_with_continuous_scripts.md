# Китайский, японский и корейский языки (CJK) и тайский язык

Manticore предоставляет встроенную поддержку индексации языков с [непрерывными скриптами](https://en.wikipedia.org/wiki/Scriptio_continua) (то есть языков, которые не используют пробелы или другие знаки между словами или предложениями). Это позволяет вам обрабатывать тексты на этих языках двумя разными способами:

<!-- example cont 1 -->
1. Точное сегментирование с использованием библиотеки [ICU](https://icu.unicode.org/). В настоящее время поддерживается только китайский.


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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'cont\' morphology = \'icu_chinese\'')
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
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'icu_chinese'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'icu_chinese'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'icu_chinese'", Some(true)).await;
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
2. Точное сегментирование с использованием библиотеки [Jieba](https://github.com/fxsjy/jieba). Как и ICU, в настоящее время она поддерживает только китайский.

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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'cont\' morphology = \'jieba_chinese\'')
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
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'jieba_chinese'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'jieba_chinese'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = 'cont' morphology = 'jieba_chinese'", Some(true)).await;
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
3. Базовая поддержка с использованием опций N-gram [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) и [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)
Для каждого языка с непрерывным скриптом существуют отдельные таблицы набора символов (`chinese`, `korean`, `japanese`, `thai`), которые можно использовать. В качестве альтернативы вы можете использовать общую таблицу набора символов `cont` для поддержки всех CJK и тайских языков одновременно или набор символов `cjk` для включения только всех CJK языков.


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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')
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
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", Some(true)).await;
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
Кроме того, есть встроенная поддержка китайских [стоп-слов](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) с алиасом `zh`.


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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'chinese\' morphology = \'icu_chinese\' stopwords = \'zh\'')
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
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'chinese' morphology = 'icu_chinese' stopwords = 'zh'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'chinese' morphology = 'icu_chinese' stopwords = 'zh'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = 'chinese' morphology = 'icu_chinese' stopwords = 'zh'", Some(true)).await;
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

