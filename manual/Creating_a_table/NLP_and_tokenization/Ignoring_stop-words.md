# Ignoring stop words

Stop words are words that are ignored during indexing and searching, typically due to their high frequency and low value to search results. 

Manticore Search applies [stemming](../../Creating_a_table/NLP_and_tokenization/Morphology.md) to stop words by default, which can lead to undesired results, but this can be turned off using the [stopwords_unstemmed](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed).

Small stop word files are stored in the table header, and there is a limit to the size of files that can be embedded, as defined by the [embedded_limit](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit) option.

Stop words are not indexed, but they do affect keyword positions. For example, if "the" is a stop word, and document 1 contains the phrase "in office" while document 2 contains the phrase "in the office," searching for "in office" as an exact phrase will only return the first document, even though "the" is skipped as a stop word in the second document. This behavior can be modified using the [stopword_step](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step) directive.

## stopwords

```ini
stopwords=path/to/stopwords/file[ path/to/another/file ...]
```

<!-- example stopwords -->
The stopwords setting is optional and by default empty. It allows you to specify the path to one or more stop word files, separated by spaces. All the files will be loaded. In the real-time mode, only absolute paths are allowed.

The stop word file format is simple plain text with UTF-8 encoding. The file data will be tokenized with respect to the [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) settings, so you can use the same separators as in the indexed data.

Stop word files can be created manually or semi-automatically. The [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) provides a mode that creates a frequency dictionary of the table, sorted by the keyword frequency. Top keywords from that dictionary can usually be used as stop words. See [--buildstops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) and [--buildfreqs](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) switch for details. Top keywords from that dictionary can usually be used as stop words.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt stopwords-ru.txt stopwords-en.txt'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => '/usr/local/manticore/data/stopwords.txt stopwords-ru.txt stopwords-en.txt'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'");
```
<!-- request CONFIG -->

```ini
table products {
  stopwords = /usr/local/manticore/data/stopwords.txt
  stopwords = stopwords-ru.txt stopwords-en.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

Alternatively you can use one of the default stop word files that come with Manticore. Currently stop words for 50 languages are available. Here is the full list of aliases for them:

* af - Afrikaans
* ar - Arabic
* bg - Bulgarian
* bn - Bengali
* ca - Catalan
* ckb- Kurdish
* cz - Czech
* da - Danish
* de - German
* el - Greek
* en - English
* eo - Esperanto
* es - Spain
* et - Estonian
* eu - Basque
* fa - Persian
* fi - Finnish
* fr - French
* ga - Irish
* gl - Galician
* hi - Hindi
* he - Hebrew
* hr - Croatian
* hu - Hungarian
* hy - Armenian
* id - Indonesian
* it - Italian
* ja - Japanese
* ko - Korean
* la - Latin
* lt - Lithuanian
* lv - Latvian
* mr - Marathi
* nl - Dutch
* no - Norwegian
* pl - Polish
* pt - Portuguese
* ro - Romanian
* ru - Russian
* sk - Slovak
* sl - Slovenian
* so - Somali
* st - Sotho
* sv - Swedish
* sw - Swahili
* th - Thai
* tr - Turkish
* yo - Yoruba
* zh - Chinese
* zu - Zulu

<!-- example stopwords 1 -->
For example, to use stop words for Italian language just put the following line in your config file:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'it'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'it'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'it'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'it\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'it\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'it'");
```
<!-- request CONFIG -->

```ini
table products {
  stopwords = it

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example stopwords 2 -->
If you need to use stop words for multiple languages you should list all their aliases, separated with commas (RT mode) or spaces (plain mode):


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'en, it, ru'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en, it, ru\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en, it, ru\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'");
```
<!-- request CONFIG -->

```ini
table products {
  stopwords = en it ru

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## stopword_step

```ini
stopword_step={0|1}
```

<!-- example stopword_step -->
The position_increment setting on [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) is optional, and the allowed values are 0 and 1, with the default being 1.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'en, it, ru',
            'stopword_step' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'");
```
<!-- request CONFIG -->

```ini
table products {
  stopwords = en
  stopword_step = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## stopwords_unstemmed

```ini
stopwords_unstemmed={0|1}
```

<!-- example stopwords_unstemmed -->
Whether to apply stop words before or after stemming. Optional, default is 0 (apply stop word filter after stemming).

By default, stop words are stemmed themselves, and then applied to tokens *after* stemming (or any other morphology processing). This means that a token is stopped when stem(token) is equal to stem(stopword). This default behavior can lead to unexpected results when a token is erroneously stemmed to a stopped root. For example, "Andes" might get stemmed to "and", so when "and" is a stopword, "Andes" is also skipped.

However, you can change this behavior by enabling the `stopwords_unstemmed` directive. When this is enabled, stop words are applied before stemming (and therefore to the original word forms), and the tokens are skipped when the token is equal to the stopword.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'en, it, ru',
            'stopwords_unstemmed' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'");
```
<!-- request CONFIG -->

```ini
table products {
  stopwords = en
  stopwords_unstemmed = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->