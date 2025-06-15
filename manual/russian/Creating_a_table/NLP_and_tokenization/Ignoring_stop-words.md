# Игнорирование стоп-слов

Стоп-слова — это слова, которые игнорируются при индексации и поиске, обычно из-за их высокой частоты и низкой значимости для результатов поиска.

Manticore Search по умолчанию применяет [стемминг](../../Creating_a_table/NLP_and_tokenization/Morphology.md) к стоп-словам, что может привести к нежелательным результатам, но это можно отключить с помощью [stopwords_unstemmed](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed).

Небольшие файлы стоп-слов хранятся в заголовке таблицы, и существует ограничение на размер файлов, которые могут быть встроены, согласно опции [embedded_limit](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit).

Стоп-слова не индексируются, но влияют на позиции ключевых слов. Например, если "the" — стоп-слово, и в документе 1 содержится фраза "in office", а в документе 2 — "in the office", поиск точной фразы "in office" вернёт только первый документ, даже несмотря на то, что "the" пропускается как стоп-слово во втором документе. Это поведение можно изменить с помощью директивы [stopword_step](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step).

## stopwords

```ini
stopwords=path/to/stopwords/file[ path/to/another/file ...]
```

<!-- example stopwords -->
Параметр stopwords является необязательным и по умолчанию пуст. Он позволяет задать путь к одному или нескольким файлам стоп-слов, разделённым пробелами. Все файлы будут загружены. В режиме реального времени разрешены только абсолютные пути.

Формат файла стоп-слов — простой текст с кодировкой UTF-8. Данные файла будут токенизированы в соответствии с настройками [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), так что можно использовать те же разделители, что и в индексируемых данных.

Файлы стоп-слов можно создавать вручную или полуавтоматически. [Индексатор](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) предоставляет режим, который создаёт частотный словарь таблицы, отсортированный по частоте ключевых слов. Верхние ключевые слова из этого словаря обычно можно использовать как стоп-слова. Для деталей смотрите переключатели [--buildstops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) и [--buildfreqs](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments). Верхние ключевые слова из этого словаря обычно можно использовать как стоп-слова.


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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'", Some(true)).await;
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

В качестве альтернативы можно использовать один из стандартных файлов стоп-слов, поставляемых с Manticore. В настоящее время доступны стоп-слова для 50 языков. Вот полный список их алиасов:

* af - африкаанс
* ar - арабский
* bg - болгарский
* bn - бенгальский
* ca - каталанский
* ckb- курдский
* cz - чешский
* da - датский
* de - немецкий
* el - греческий
* en - английский
* eo - эсперанто
* es - испанский
* et - эстонский
* eu - баскский
* fa - персидский
* fi - финский
* fr - французский
* ga - ирландский
* gl - галисийский
* hi - хинди
* he - иврит
* hr - хорватский
* hu - венгерский
* hy - армянский
* id - индонезийский
* it - итальянский
* ja - японский
* ko - корейский
* la - латинский
* lt - литовский
* lv - латвийский
* mr - маратхи
* nl - нидерландский
* no - норвежский
* pl - польский
* pt - португальский
* ro - румынский
* ru - русский
* sk - словацкий
* sl - словенский
* so - сомалийский
* st - сото
* sv - шведский
* sw - суахили
* th - тайский
* tr - турецкий
* yo - йоруба
* zh - китайский
* zu - зулу

<!-- example stopwords 1 -->
Например, чтобы использовать стоп-слова для итальянского языка, просто добавьте следующую строку в ваш конфигурационный файл:


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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'it\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'it\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'it'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = 'it'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = 'it'", Some(true)).await;
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
Если нужно использовать стоп-слова для нескольких языков, перечислите все их алиасы, разделённые запятыми (в режиме RT) или пробелами (в обычном режиме):


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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en, it, ru\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en, it, ru\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'", Some(true)).await;
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
Параметр position_increment для [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) является необязательным, допустимые значения — 0 и 1, значение по умолчанию — 1.


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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'", Some(true)).await;
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
Применять стоп-слова до или после стемминга. Необязательно, по умолчанию 0 (фильтр стоп-слов применяется после стемминга).

По умолчанию стоп-слова сами подвергаются стеммингу, а затем применяются к токенам *после* стемминга (или другой морфологической обработки). Это означает, что токен отбрасывается, когда stem(token) равен stem(stopword). Такое поведение по умолчанию может приводить к неожиданным результатам, когда токен ошибочно стеммится до стоп-слова. Например, "Andes" может быть стеммировано как "and", и если "and" является стоп-словом, то "Andes" тоже будет исключено.

Однако вы можете изменить это поведение, включив директиву `stopwords_unstemmed`. При её активации стоп-слова применяются до стемминга (т.е. к исходным формам слов), и токены отбрасываются, когда токен равен стоп-слову.

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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'", Some(true)).await;
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

