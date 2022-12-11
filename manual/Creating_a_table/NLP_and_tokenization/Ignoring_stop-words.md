# Ignoring stop words

Stop words are the words that are skipped during indexing and searching. Typically you'd put most frequent words to the stop words list, because they do not add much value to search results but consume a lot of resources to process.

[Stemming](../../Creating_a_table/NLP_and_tokenization/Morphology.md) is by default applied when parsing stop words file. That might however lead to undesired results. You can turn that off with [stopwords_unstemmed](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed).

Small enough files are stored in the table header, see [embedded_limit](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit) for details.

While stop words are not indexed, they still do affect the keyword positions. For instance, assume that "the" is a stop word, that document 1 contains the line "in office", and that document 2 contains "in the office". Searching for "in office" as for an exact phrase will only return the first document, as expected, even though "the" in the second one is skipped as a stop word. That behavior can be tweaked through the [stopword_step](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step) directive.

## stopwords

```ini
stopwords=path/to/stopwords/file[ path/to/another/file ...]
```

<!-- example stopwords -->
Stop word files list (space separated). Optional, default is empty. You can specify several file names, separated by spaces. All the files will be loaded. In the RT mode only absolute paths are allowed.

Stop words file format is simple plain text. The encoding must be UTF-8. File data will be tokenized with respect to [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) settings, so you can use the same separators as in the indexed data.

Stop word files can either be created manually, or semi-automatically. [indexer](../../Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) provides a mode that creates a frequency dictionary of the table, sorted by the keyword frequency, see [--buildstops](../../Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) and [--buildfreqs](../../Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) switch for details. Top keywords from that dictionary can usually be used as stop words.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'
```

<!-- request HTTP -->

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

<!-- request HTTP -->

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

<!-- request HTTP -->

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
Position increment on [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords). Optional, allowed values are 0 and 1, default is 1.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'
```

<!-- request HTTP -->

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

By default, stop words are stemmed themselves, and applied to tokens *after* stemming (or any other morphology processing). In other words, by default, a token is stopped when stem(token) is equal to stem(stopword). That can lead to unexpected results when a token gets (erroneously) stemmed to a stopped root. For example, 'Andes' might get stemmed to 'and', so when 'and' is a stopword, 'Andes' is also skipped.

stopwords_unstemmed directive changed this behaviour. When it's enabled, stop words are applied before stemming (and therefore to the original word forms), and the tokens are skipped when token is equal to stopword.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'
```

<!-- request HTTP -->

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
