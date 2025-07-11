# Продвинутая морфология

Морфологические препроцессоры могут применяться к словам во время индексирования для нормализации различных форм одного и того же слова и улучшения сегментации. Например, английский стеммер может нормализовать "dogs" и "dog" в "dog", что приведет к одинаковым результатам поиска для обоих ключевых слов.

В Manticore есть четыре встроенных морфологических препроцессора:

*   **Лемматизатор**: сводит слово к корню или лемме. Например, "running" может быть приведено к "run", а "octopi" к "octopus". Следует отметить, что некоторые слова могут иметь несколько соответствующих корневых форм. Например, "dove" может быть как прошедшим временем от "dive", так и существительным, означающим птицу, как в "A white dove flew over the cuckoo's nest." В таком случае лемматизатор может сгенерировать все возможные корневые формы.
*   **Стеммер**: сводит слово к стему, удаляя или заменяя определённые известные суффиксы. Полученный стем не обязательно является валидным словом. Например, английский стеммер Портера сводит "running" к "run", "business" к "busi" (невалидное слово), и совсем не изменяет "octopi".
*   **Фонетические алгоритмы**: заменяют слова фонетическими кодами, которые одинаковы, даже если слова разные, но звучат похоже.
*   **Алгоритмы разбиения слов**: разбивают текст на слова. В настоящее время доступны только для китайского языка.

## morphology

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->
Директива morphology указывает список морфологических препроцессоров, которые нужно применить к индексируемым словам. Это необязательная настройка, по умолчанию препроцессоры не применяются.

В Manticore встроены морфологические препроцессоры для:

* английского, русского и немецкого языков (лемматизаторы)
* английского, русского, арабского и чешского языков (стеммеры)
* фонетических алгоритмов SoundEx и MetaPhone
* алгоритма разбиения китайских слов
* Стеммеров Snowball (libstemmer) для более чем [15 других языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) также доступны.

Для лемматизаторов требуются словарные файлы `.pak`, которые можно установить через пакеты `manticore-language-packs` или [скачать с сайта Manticore](https://manticoresearch.com/install/#other-downloads). В последнем случае словари необходимо поместить в директорию, указанную в настройке [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base).

Кроме того, настройка [lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) может использоваться для ускорения лемматизации за счет выделения большего объема оперативной памяти под кэш не сжатого словаря.

Сегментация китайского языка может выполняться с помощью [ICU](http://site.icu-project.org/) или [Jieba](https://github.com/yanyiwu/cppjieba) (требует пакет `manticore-language-packs`). Обе библиотеки обеспечивают более точную сегментацию, чем n-grams, но работают немного медленнее. В [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) должны быть включены все китайские символы, это можно сделать с помощью наборов символов `cont`, `cjk` или `chinese`. При установке `morphology=icu_chinese` или `morphology=jieba_chinese` документы сначала обрабатываются ICU или Jieba, затем результат обрабатывается токенизатором в соответствии с charset_table, и только после этого применяются другие морфологические процессоры из опции `morphology`. На сегментацию ICU/Jieba передаются только те части текста, которые содержат китайские символы, в то время как остальные части могут обрабатываться другими способами, например, другими морфологиями или `charset_table`.

Встроенные английский и русский стеммеры работают быстрее, чем их аналоги из libstemmer, но могут давать немного отличающиеся результаты.

Реализация Soundex совпадает с MySQL. Реализация Metaphone основана на алгоритме Double Metaphone и индексирует первичный код.

Чтобы использовать опцию `morphology`, укажите одну или несколько из встроенных опций, включая:
* none: не выполнять морфологическую обработку
* lemmatize_ru - применить русский лемматизатор и выбрать одну корневую форму
* lemmatize_uk - применить украинский лемматизатор и выбрать одну корневую форму (сначала установите его в [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) или [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer)). Для корректной работы лемматизатора убедитесь, что специфические украинские символы сохранены в вашем `charset_table`, так как по умолчанию их нет. Для этого переопределите их, например так: `charset_table='non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`. [Здесь](https://play.manticoresearch.com/ua-lemmatizer/) находится интерактивный курс по установке и использованию украинского лемматизатора.
* lemmatize_en - применить английский лемматизатор и выбрать одну корневую форму
* lemmatize_de - применить немецкий лемматизатор и выбрать одну корневую форму
* lemmatize_ru_all - применить русский лемматизатор и индексировать все возможные корневые формы
* lemmatize_uk_all - применить украинский лемматизатор и индексировать все возможные корневые формы. Ссылки на установку приведены выше, обратите внимание на `charset_table`.
* lemmatize_en_all - применить английский лемматизатор и индексировать все возможные корневые формы
* lemmatize_de_all - применить немецкий лемматизатор и индексировать все возможные корневые формы
* stem_en - применить английский стеммер Портера
* stem_ru - применить русский стеммер Портера
* stem_enru - применить английский и русский стеммеры Портера
* stem_cz - применить чешский стеммер
* stem_ar - применить арабский стеммер
* soundex - заменить ключевые слова их SOUNDEX-кодом
* metaphone - заменить ключевые слова их METAPHONE-кодом
* icu_chinese - применить сегментацию китайского текста через ICU
* jieba_chinese - применить сегментацию китайского текста через Jieba (требуется пакет `manticore-language-packs`)
* libstemmer_* . Смотрите [список поддерживаемых языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) для подробностей

Несколько стеммеров могут быть указаны через запятую. Они будут применяться к входящим словам в порядке их перечисления, и обработка остановится, как только один из стеммеров изменит слово. Дополнительно, когда включена функция [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms), слово сначала будет искаться в словаре словоформ. Если в словаре найдена соответствующая запись, стеммеры применяться не будут. [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) можно использовать для реализации исключений при стемминге.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'
```

<!-- request JSON -->

```json
POST /cli -d "CREATE TABLE products(title text, price float)  morphology = 'stem_en, libstemmer_sv'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'morphology' => 'stem_en, libstemmer_sv'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'stem_en, libstemmer_sv\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'stem_en, libstemmer_sv\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'stem_en, libstemmer_sv\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  morphology = stem_en, libstemmer_sv

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## morphology_skip_fields

<!-- example morphology_skip_fields -->

```ini
morphology_skip_fields = field1[, field2, ...]
```

Список полей, для которых следует пропустить предварительную обработку морфологии. Опционально, по умолчанию пусто (применять препроцессоры ко всем полям).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, name text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, name text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'morphology_skip_fields' => 'name',
            'morphology' => 'stem_en'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = \'name\' morphology = \'stem_en\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = \'name\' morphology = \'stem_en\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = \'name\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  morphology_skip_fields = name
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_field = name
  rt_attr_uint = price
}
```
<!-- end -->

## min_stemming_len

<!-- example min_stemming_len -->

```ini
min_stemming_len = length
```

Минимальная длина слова, при которой включается стемминг. Опционально, по умолчанию 1 (стемминг для всех слов).

Стеммеры не идеальны и иногда могут давать нежелательные результаты. Например, пропуск ключевого слова "gps" через стеммер Porter для английского приводит к "gp", что фактически не является целью. Функция `min_stemming_len` позволяет подавить стемминг в зависимости от длины исходного слова, то есть чтобы не стеммить слишком короткие слова. Ключевые слова, короче заданного порога, стеммиться не будут. Обратите внимание, что ключевые слова, длина которых точно равна указанному значению, **будут** стеммиться. Так что, чтобы избежать стемминга ключевых слов из 3 символов, нужно указать значение 4. Для более тонкой настройки смотрите функцию [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'min_stemming_len' => '4',
             'morphology' => 'stem_en'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = \'4\' morphology = \'stem_en\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asycnio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = \'4\' morphology = \'stem_en\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = \'4\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  min_stemming_len = 4
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## index_exact_words

<!-- example index_exact_words -->

```ini
index_exact_words = {0|1}
```

Эта опция позволяет индексировать исходные ключевые слова вместе с их морфологически изменёнными версиями. Однако исходные ключевые слова, которые перенаправляются [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) и [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md), индексироваться не могут. Значение по умолчанию — 0, что означает, что эта функция отключена.

Это позволяет использовать [оператор точной формы](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier) в языке запросов. Включение этой функции увеличит размер полнотекстового индекса и время индексации, но не повлияет на производительность поиска.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'index_exact_words' => '1',
             'morphology' => 'stem_en'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = \'1\' morphology = \'stem_en\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = \'1\' morphology = \'stem_en\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = \'1\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  index_exact_words = 1
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_hmm

<!-- example jieba_hmm -->

```ini
jieba_hmm = {0|1}
```

Включить или отключить HMM в инструменте сегментации Jieba. Опционально; значение по умолчанию — 1.

В Jieba опция HMM (Hidden Markov Model, скрытая марковская модель) относится к алгоритму, используемому для сегментации слов. В частности, она позволяет Jieba выполнять сегментацию китайских слов, распознавая неизвестные слова, особенно отсутствующие в словаре.

Jieba в основном использует метод на основе словаря для сегментации известных слов, но при включённой опции HMM применяется статистическая модель для определения вероятных границ слов для слов или фраз, отсутствующих в словаре. Это особенно полезно для сегментации новых или редких слов, имён и сленга.

Таким образом, опция `jieba_hmm` помогает улучшить точность сегментации за счёт производительности индексации. Она должна использоваться вместе с `morphology = jieba_chinese`, см. [Китайский, японский, корейский (CJK) и тайский языки](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
			 'morphology' => 'jieba_chinese',
	  		 'jieba_hmm'='1'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_hmm = \'0\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_hmm = \'0\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_hmm = \'0\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_hmm = 0

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_mode

<!-- example jieba_mode -->

```ini
jieba_mode = {accurate|full|search}
```

Режим сегментации Jieba. Опционально; по умолчанию — `accurate`.

В режиме точного разбиения Jieba делит предложение на максимально точные слова, используя сопоставление со словарём. Этот режим ориентирован на точность, обеспечивая максимально корректное разделение.

В полном режиме Jieba пытается разбить предложение на все возможные комбинации слов, стремясь включить все потенциальные слова. Этот режим ориентирован на максимальный охват, то есть идентифицирует как можно больше слов, даже если некоторые из них пересекаются или используются реже. Он возвращает все слова, найденные в его словаре.

В режиме поиска Jieba разбивает текст на целые слова и меньшие части, сочетая точное разделение с дополнительной детализацией за счёт предоставления пересекающихся фрагментов слов. Этот режим балансирует точность и охват, что полезно для поисковых систем.

`jieba_mode` должен использоваться вместе с `morphology = jieba_chinese`. См. [Китайские, японские, корейские (CJK) и тайские языки](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
			 'morphology' => 'jieba_chinese',
	  		 'jieba_mode'='full'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_mode = \'full\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_mode = \'full\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_mode = \'full\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_mode = full

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_user_dict_path

<!-- example jieba_user_dict_path -->

```ini
jieba_user_dict_path = path/to/stopwords/file
```

Путь к пользовательскому словарю Jieba. Опционально.

Jieba, библиотека для сегментации китайского текста, использует словарные файлы для помощи в разбиении слов. Формат этих словарных файлов следующий: каждая строка содержит слово, разделённое на три части пробелами — само слово, частота слова и тег части речи (POS). Частота слова и POS-тег являются необязательными и могут быть опущены. Файл словаря должен быть в кодировке UTF-8.

Пример:

```
创新办 3 i
云计算 5
凱特琳 nz
台中
```

`jieba_user_dict_path` должен использоваться вместе с `morphology = jieba_chinese`. Для подробностей см. [Китайские, японские, корейские (CJK) и тайские языки](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
			 'morphology' => 'jieba_chinese',
             'jieba_user_dict_path' = '/usr/local/manticore/data/user-dict.txt'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_user_dict_path = \'/usr/local/manticore/data/user-dict.txt\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_user_dict_path = \'/usr/local/manticore/data/user-dict.txt\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_user_dict_path = \'/usr/local/manticore/data/user-dict.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_user_dict_path = /usr/local/manticore/data/user-dict.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->

