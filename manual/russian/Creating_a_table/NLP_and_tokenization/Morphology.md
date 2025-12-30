# Продвинутая морфология

Морфологические препроцессоры могут применяться к словам во время индексации для нормализации разных форм одного и того же слова и улучшения сегментации. Например, английский стеммер может нормализовать "dogs" и "dog" в "dog", что приводит к одинаковым результатам поиска по обоим ключевым словам.

В Manticore есть четыре встроенных морфологических препроцессора:

*   **Лемматизатор**: сводит слово к корню или лемме. Например, "running" может быть приведено к "run", а "octopi" — к "octopus". Обратите внимание, что у некоторых слов может быть несколько соответствующих корневых форм. Например, "dove" может быть либо прошедшей формой глагола "dive", либо существительным, обозначающим птицу, как в предложении "A white dove flew over the cuckoo's nest." В таком случае лемматизатор может сгенерировать все возможные корневые формы.
*   **Стеммер**: сводит слово к основе, удаляя или заменяя известные суффиксы. Полученная основа может не быть валидным словом. Например, английский стеммер Портера сводит "running" к "run", "business" к "busi" (не валидное слово) и вообще не изменяет "octopi".
*   **Фонетические алгоритмы**: заменяют слова на фонетические коды, которые одинаковы, даже если слова различаются, но звучат похоже.
*   **Алгоритмы разбиения слов**: разбивают текст на слова. В настоящее время доступны только для китайского языка.

## morphology

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->
Директива morphology задаёт список морфологических препроцессоров, применяемых к индексируемым словам. Это необязательная настройка, по умолчанию препроцессоры не применяются.

Manticore включает встроенные морфологические препроцессоры для:

* английского, русского и немецкого языков (лемматизаторы)
* английского, русского, арабского и чешского (стеммеры)
* фонетических алгоритмов SoundEx и MetaPhone
* алгоритма разбиения слов для китайского языка
* стеммеров Snowball (libstemmer) более чем для [15 других языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md).

Лемматизаторы требуют словарных файлов `.pak`, которые можно установить с помощью пакетов `manticore-language-packs` или [скачать с сайта Manticore](https://manticoresearch.com/install/#other-downloads). В последнем случае словари нужно положить в директорию, указанную в настройке [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base).

Кроме того, настройка [lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) может быть использована для ускорения лемматизации за счёт выделения большего объёма оперативной памяти для кэша распакованного словаря.

Сегментация языка китайского может выполняться с помощью [ICU](http://site.icu-project.org/) или [Jieba](https://github.com/yanyiwu/cppjieba) (требуется пакет `manticore-language-packs`). Обе библиотеки обеспечивают более точную сегментацию, чем n-граммы, но работают немного медленнее. В [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) должны быть включены все китайские символы, что можно сделать с помощью наборов символов `cont`, `cjk` или `chinese`. При установке `morphology=icu_chinese` или `morphology=jieba_chinese` документы сначала обрабатываются ICU или Jieba. Затем токенизатор обрабатывает результат в соответствии с charset_table и, наконец, применяются другие морфологические препроцессоры из опции `morphology`. Для сегментации ICU/Jieba передаются только те части текста, которые содержат китайский язык, тогда как другие части могут быть изменены другими способами, например, различными морфологиями или с помощью `charset_table`.

Встроенные английский и русский стеммеры работают быстрее своих аналогов из libstemmer, но могут выдавать немного отличающиеся результаты.

Реализация Soundex совпадает с реализацией в MySQL. Реализация Metaphone основана на алгоритме Double Metaphone и индексирует первичный код.

Для использования опции `morphology` укажите один или несколько из встроенных вариантов, в том числе:
* none: не применять морфологическую обработку
* lemmatize_ru - применять русский лемматизатор и выбирать одну корневую форму
* lemmatize_uk - применять украинский лемматизатор и выбирать одну корневую форму (сначала установите его в [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) или [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer)). Для корректной работы лемматизатора убедитесь, что специфические украинские символы сохранены в вашем `charset_table`, так как по умолчанию они отсутствуют. Для этого переопределите их, например: `charset_table='non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`. [Здесь](https://play.manticoresearch.com/ua-lemmatizer/) находится интерактивный курс по установке и использованию украинского лемматизатора.
* lemmatize_en - применять английский лемматизатор и выбирать одну корневую форму
* lemmatize_de - применять немецкий лемматизатор и выбирать одну корневую форму
* lemmatize_ru_all - применять русский лемматизатор и индексировать все возможные корневые формы
* lemmatize_uk_all - применять украинский лемматизатор и индексировать все возможные корневые формы. Ссылки на установку приведены выше, а также обратите внимание на `charset_table`.
* lemmatize_en_all - применять английский лемматизатор и индексировать все возможные корневые формы
* lemmatize_de_all - применять немецкий лемматизатор и индексировать все возможные корневые формы
* stem_en - применять английский стеммер Портера
* stem_ru - применять русский стеммер Портера
* stem_enru - применять английский и русский стеммеры Портера
* stem_cz - применять чешский стеммер
* stem_ar - применять арабский стеммер
* soundex - заменять ключевые слова их SOUNDEX-кодом
* metaphone - заменять ключевые слова их METAPHONE-кодом
* icu_chinese - применять сегментацию китайского текста с использованием ICU
* jieba_chinese - применять сегментацию китайского текста с использованием Jieba (требуется пакет `manticore-language-packs`)
* libstemmer_* . Смотрите список поддерживаемых языков в [списке поддерживаемых языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) для детализации

Несколько стеммеров могут быть указаны, разделённые запятыми. Они будут применяться к входящим словам в том порядке, в котором перечислены, и обработка остановится, как только один из стеммеров изменит слово. Кроме того, при включённой функции [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms), сначала будет осуществлён поиск слова в словаре словоформ. Если в словаре найдётся совпадающая запись, применение стеммеров не происходит. Функция [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) может использоваться для реализации исключений стемминга.

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

Список полей, для которых пропускается морфологическая предобработка. Необязательно, по умолчанию пустой (применять препроцессоры ко всем полям).

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

Минимальная длина слова, с которой включается стемминг. Необязательно, по умолчанию 1 (стемминг для всех слов).

Стеммеры не идеальны, и иногда могут давать нежелательные результаты. Например, пропуск ключевого слова "gps" через Porter стеммер для английского приводит к "gp", что не соответствует намерению. Опция `min_stemming_len` позволяет подавить стемминг в зависимости от длины исходного слова, то есть избежать стемминга слишком коротких слов. Ключевые слова, короче указанного порога, не будут подвергаться стеммингу. Обратите внимание, что ключевые слова точно такой длины **будут** обработаны стеммерами. Таким образом, чтобы избежать стемминга трёхсимвольных ключевых слов, следует указать значение 4. Для более тонкой настройки обратитесь к функции [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms).

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

Эта опция позволяет индексировать как оригинальные ключевые слова, так и их морфологически изменённые версии. Однако оригинальные ключевые слова, переназначенные через [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) и [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md), не могут быть проиндексированы. Значение по умолчанию — 0, что означает отключение этой функции.

Это даёт возможность использовать [оператор точной формы](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier) в языке запросов. Включение этой функции увеличит размер полного текстового индекса и время индексации, но не повлияет на производительность поиска.

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

Включить или отключить HMM в инструменте сегментации Jieba. Необязательно; значение по умолчанию — 1.

В Jieba опция HMM (скрытая марковская модель) относится к алгоритму, используемому для сегментации слов. В частности, она позволяет Jieba выполнять сегментацию китайских слов, распознавая неизвестные слова, особенно отсутствующие в его словаре.

Jieba в основном использует словарный метод для сегментации известных слов, но при включении опции HMM применяется статистическая модель для определения вероятных границ слов для слов или фраз, отсутствующих в словаре. Это особенно полезно для сегментации новых или редких слов, имён и сленга.

Вкратце, опция `jieba_hmm` помогает улучшить точность сегментации за счёт снижения производительности индексации. Её необходимо использовать вместе с `morphology = jieba_chinese`, см. [Китайский, японский и корейский (CJK) и тайский языки](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

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

В режиме точности (accurate) Jieba разбивает предложение на максимально точные слова, используя сопоставление со словарём. Этот режим ориентирован на точность, обеспечивая наиболее аккуратное разбиение.

В полном режиме (full) Jieba пытается разбить предложение на все возможные словосочетания, стараясь включить все потенциальные слова. Этот режим акцентирует внимание на полноте, то есть выявляет как можно больше слов, даже если некоторые из них перекрываются или используются реже. Возвращаются все слова, найденные в словаре.

В режиме поиска (search) Jieba делит текст как на целые слова, так и на меньшие части, сочетая точное сегментирование с дополнительной детализацией за счет перекрывающихся фрагментов слов. Этот режим балансирует между точностью и полнотой, что полезно для поисковых систем.

`jieba_mode` должен использоваться вместе с `morphology = jieba_chinese`. См. [Китайский, японский, корейский (CJK) и тайский языки](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

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

Jieba — библиотека сегментации китайского текста, использующая словарные файлы для помощи с разбиением слов. Формат этих словарных файлов следующий: каждая строка содержит слово, разделённое на три части пробелами — само слово, частоту слова и тег части речи (POS). Частота слова и тег части речи являются необязательными и могут быть опущены. Файл словаря должен быть в кодировке UTF-8.

Пример:

```
创新办 3 i
云计算 5
凱特琳 nz
台中
```

`jieba_user_dict_path` должен использоваться вместе с `morphology = jieba_chinese`. Подробнее см. [Китайский, японский, корейский (CJK) и тайский языки](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

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

