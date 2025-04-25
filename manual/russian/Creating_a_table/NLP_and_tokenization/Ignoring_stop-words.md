# Игнорирование стоп-слов

Стоп-слова — это слова, которые игнорируются при индексировании и поиске, обычно из-за их высокой частоты и низкой ценности для поисковых результатов. 

Manticore Search применяет [стемминг](../../Creating_a_table/NLP_and_tokenization/Morphology.md) к стоп-словам по умолчанию, что может привести к нежелательным результатам, но это можно отключить с помощью [stopwords_unstemmed](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed).

Малые файлы стоп-слов хранятся в заголовке таблицы, и существует ограничение на размер файлов, которые могут быть встроены, как определено параметром [embedded_limit](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit).

Стоп-слова не индексируются, но они влияют на позиции ключевых слов. Например, если "the" является стоп-словом, и документ 1 содержит фразу "in office", в то время как документ 2 содержит фразу "in the office", поиск "in office" как точной фразы вернёт только первый документ, даже несмотря на то, что "the" пропущено как стоп-слово во втором документе. Это поведение можно изменить с помощью директивы [stopword_step](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step).

## стоп-слова

```ini
stopwords=path/to/stopwords/file[ path/to/another/file ...]
```

<!-- example stopwords -->
Настройка стоп-слов является необязательной и по умолчанию пуста. Она позволяет вам указать путь к одному или нескольким файлам стоп-слов, разделённым пробелами. Все файлы будут загружены. В режиме реального времени разрешены только абсолютные пути.

Формат файла стоп-слов — это простой текст в кодировке UTF-8. Данные файла будут токенизированы с учетом настроек [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), так что вы можете использовать те же разделители, что и в индексированных данных.

Файлы стоп-слов могут быть созданы вручную или полудоступно. [Индексатор](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) предоставляет режим, который создает частотный словарь таблицы, отсортированный по частоте ключевых слов. Топ-ключевые слова из этого словаря могут обычно использоваться как стоп-слова. См. параметры [--buildstops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) и [--buildfreqs](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) для получения подробной информации. Топ-ключевые слова из этого словаря могут обычно использоваться как стоп-слова.

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'');
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

Кроме того, вы можете использовать один из стандартных файлов стоп-слов, которые поставляются с Manticore. В настоящее время доступны стоп-слова для 50 языков. Вот полный список их псевдонимов:

* af - Африкаанс
* ar - Арабский
* bg - Болгарский
* bn - Бенгальский
* ca - Каталонский
* ckb- Курдский
* cz - Чешский
* da - Датский
* de - Немецкий
* el - Греческий
* en - Английский
* eo - Эсперанто
* es - Испанский
* et - Эстонский
* eu - Баскский
* fa - Персидский
* fi - Финский
* fr - Французский
* ga - Ирландский
* gl - Галисийский
* hi - Хинди
* he - Иврит
* hr - Хорватский
* hu - Венгерский
* hy - Армянский
* id - Индонезийский
* it - Итальянский
* ja - Японский
* ko - Корейский
* la - Латинский
* lt - Литовский
* lv - Латышский
* mr - Маратхи
* nl - Голландский
* no - Норвежский
* pl - Польский
* pt - Португальский
* ro - Румынский
* ru - Русский
* sk - Словацкий
* sl - Словенский
* so - Сомалийский
* st - Сото
* sv - Шведский
* sw - Суахили
* th - Тайский
* tr - Турецкий
* yo - Йоруба
* zh - Китайский
* zu - Зулу
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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'it'')
```
<!-- intro -->
##### Python-asyncio:
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'it'')
```
<!-- intro -->
##### Javascript:
<!-- request javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'it'');
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
Если вам нужно использовать стоп-слова для нескольких языков, вы должны перечислить все их псевдонимы через запятые (режим RT) или пробелы (обычный режим):
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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'')
```
<!-- intro -->
##### Python-asyncio:
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'')
```
<!-- intro -->
##### Javascript:
<!-- request javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'');
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
Настройка position_increment для [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) является необязательной, и допустимые значения - 0 и 1, по умолчанию используется 1.
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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'')
```
<!-- intro -->
##### Python-asyncio:
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'", Some(true)).await;
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

<!-- пример stopwords_unstemmed -->
Нужно ли применять стоп-слова до или после стемминга. Необязательно, по умолчанию 0 (применять фильтр стоп-слов после стемминга).

По умолчанию стоп-слова также подлежат стеммингу, а затем применяются к токенам *после* стемминга (или любой другой морфологической обработки). Это означает, что токен игнорируется, когда stem(token) равен stem(stopword). Такое поведение по умолчанию может привести к неожиданным результатам, когда токен ошибочно стеммируется в остановленный корень. Например, "Andes" может быть стеммировано в "and", поэтому, когда "and" является стоп-словом, "Andes" также пропускается.

Однако вы можете изменить это поведение, включив директиву `stopwords_unstemmed`. Когда это включено, стоп-слова применяются до стемминга (и, следовательно, к оригинальным формам слов), и токены пропускаются, когда токен равен стоп-слову.

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'", Some(true)).await;
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



































































# Ignoring stop words

Стоп-слова — это слова, которые игнорируются при индексировании и поиске, обычно из-за их высокой частотности и низкой полезности для результатов поиска. 


Manticore Search по умолчанию применяет [stemming](../../Creating_a_table/NLP_and_tokenization/Morphology.md) к стоп-словам, что может приводить к нежелательным результатам, но эту функцию можно отключить с помощью [stopwords_unstemmed](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed).


Небольшие файлы со стоп-словами хранятся в заголовке таблицы, и существует ограничение на размер файлов, которые могут быть встроены, как определено опцией [embedded_limit](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit).


Стоп-слова не индексируются, но они влияют на позиции ключевых слов. Например, если "the" является стоп-словом, и документ 1 содержит фразу "in office", а документ 2 содержит фразу "in the office", то поиск по точной фразе "in office" вернет только первый документ, даже если "the" пропускается как стоп-слово во втором документе. Это поведение можно изменить с помощью директивы [stopword_step](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step).


## stopwords

```ini
stopwords=path/to/stopwords/file[ path/to/another/file ...]
```

<!-- example stopwords -->
Параметр stopwords является необязательным и по умолчанию пуст. Он позволяет указать путь к одному или нескольким файлам со стоп-словами, разделенных пробелами. Все файлы будут загружены. В режиме реального времени допускаются только абсолютные пути.

Формат файла со стоп-словами — это простой текст в кодировке UTF-8. Данные файла будут токенизироваться с учетом настроек [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), так что вы можете использовать те же разделители, что и в индексируемых данных.

Файлы со стоп-словами можно создавать вручную или полуавтоматически. [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) предоставляет режим, который создает частотный словарь таблицы, отсортированный по частоте ключевых слов. Топ-ключевые слова из этого словаря обычно могут использоваться в качестве стоп-слов. См. переключатели [--buildstops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) и [--buildfreqs](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) для подробностей. Топ-ключевые слова из этого словаря обычно могут использоваться в качестве стоп-слов.


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'");
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

В качестве альтернативы вы можете использовать один из файлов со стоп-словами по умолчанию, поставляемых с Manticore. В настоящее время стоп-слова доступны для 50 языков. Ниже приведён полный список их псевдонимов:

* af - африкаанс
* ar - арабский
* bg - болгарский
* bn - бенгальский
* ca - каталонский
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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'it\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'it'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = 'it'");
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
Если вам нужно использовать стоп-слова для нескольких языков, вы должны перечислить все их псевдонимы, разделенные запятыми (режим RT) или пробелами (обычный режим):


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en, it, ru\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
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
Параметр position_increment в [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) является необязательным, и допустимые значения – 0 и 1, по умолчанию 1.


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
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
Применять ли стоп-слова до или после стемминга. Необязательно, по умолчанию 0 (применять фильтр стоп-слов после стемминга).

По умолчанию стоп-слова сначала стеммируются, а затем применяются к токенам *после* стемминга (или другого морфологического анализа). Это означает, что токен останавливается, когда stem(token) равен stem(stopword). Это поведение по умолчанию может привести к неожиданным результатам, когда токен ошибочно стеммируется в остановленный корень. Например, "Анды" могут быть стеммированы в "и", так что когда "и" является стоп-словом, "Анды" также пропускаются.
Однако вы можете изменить это поведение, включив директиву `stopwords_unstemmed`. Когда это включено, стоп-слова применяются перед стеммингом (и, следовательно, к оригинальным формам слов), и токены пропускаются, когда токен равен стоп-словам.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'");
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
