# Расширенная морфология

Морфологические препроцессоры могут применяться к словам во время индексирования для нормализации различных форм одного слова и улучшения сегментации. Например, английский стеммер может нормализовать "dogs" и "dog" к "dog", что приводит к идентичным результатам поиска для обоих ключевых слов.

Manticore имеет четыре встроенных морфологических препроцессора:

*   **Лемматизатор**: сводит слово к его корню или лемме. Например, "running" может быть сведено к "run", а "octopi" — к "octopus". Обратите внимание, что некоторые слова могут иметь несколько соответствующих корневых форм. Например, "dove" может быть либо прошедшим временем от "dive", либо существительным, означающим птицу, как в "A white dove flew over the cuckoo's nest." В этом случае лемматизатор может сгенерировать все возможные корневые формы.
*   **Стеммер**: сводит слово к его основе, удаляя или заменяя определенные известные суффиксы. Полученная основа не обязательно может быть допустимым словом. Например, английский стеммер Портера сводит "running" к "run", "business" к "busi" (недопустимое слово) и вообще не сводит "octopi".
*   **Фонетические алгоритмы**: заменяют слова фонетическими кодами, которые одинаковы, даже если слова разные, но фонетически близки.
*   **Алгоритмы разбиения на слова**: разделяют текст на слова. В настоящее время доступны только для китайского языка.

## morphology

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->
Директива morphology задает список морфологических препроцессоров, применяемых к индексируемым словам. Это необязательная настройка, по умолчанию препроцессор не применяется.

Manticore включает встроенные морфологические препроцессоры для:

* Лемматизаторов английского, русского и немецкого языков
* Стеммеров английского, русского, арабского и чешского языков
* Фонетических алгоритмов SoundEx и MetaPhone
* Алгоритма разбиения на слова для китайского языка
* Стеммеров Snowball (libstemmer) для более чем [15 других языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) также доступны.

Лемматизаторы требуют файлы словарей `.pak`, которые можно установить с помощью пакетов `manticore-language-packs` или [скачать с сайта Manticore](https://manticoresearch.com/install/#other-downloads). В последнем случае словари необходимо поместить в каталог, указанный в [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base).

Дополнительно, настройка [lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) может использоваться для ускорения лемматизации за счет использования большего объема ОЗУ для кэша несжатого словаря.

Сегментация китайского языка может выполняться с использованием [ICU](http://site.icu-project.org/) или [Jieba](https://github.com/yanyiwu/cppjieba) (требует пакет `manticore-language-packs`). Обе библиотеки обеспечивают более точную сегментацию, чем n-граммы, но работают немного медленнее. [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) должна включать все китайские иероглифы, что можно сделать с помощью наборов символов `cont`, `cjk` или `chinese`. Когда вы устанавливаете `morphology=icu_chinese` или `morphology=jieba_chinese`, документы сначала обрабатываются ICU или Jieba. Затем токенизатор обрабатывает результат в соответствии с charset_table, и, наконец, применяются другие морфологические процессоры из опции `morphology`. Только те части текста, которые содержат китайский язык, передаются в ICU/Jieba для сегментации, в то время как другие части могут быть изменены другими средствами, такими как различные морфологии или `charset_table`.

Встроенные английский и русский стеммеры работают быстрее, чем их аналоги в libstemmer, но могут давать несколько иные результаты.

Реализация Soundex соответствует реализации в MySQL. Реализация Metaphone основана на алгоритме Double Metaphone и индексирует первичный код.

Чтобы использовать опцию `morphology`, укажите один или несколько встроенных вариантов, включая:
* none: не выполнять никакой морфологической обработки
* lemmatize_ru - применить русский лемматизатор и выбрать одну корневую форму
* lemmatize_uk - применить украинский лемматизатор и выбрать одну корневую форму
* lemmatize_en - применить английский лемматизатор и выбрать одну корневую форму
* lemmatize_de - применить немецкий лемматизатор и выбрать одну корневую форму
* lemmatize_ru_all - применить русский лемматизатор и индексировать все возможные корневые формы
* lemmatize_uk_all - применить украинский лемматизатор и индексировать все возможные корневые формы
* lemmatize_en_all - применить английский лемматизатор и индексировать все возможные корневые формы
* lemmatize_de_all - применить немецкий лемматизатор и индексировать все возможные корневые формы
* stem_en - применить английский стеммер Портера
* stem_ru - применить русский стеммер Портера
* stem_enru - применить английский и русский стеммеры Портера
* stem_cz - применить чешский стеммер
* stem_ar - применить арабский стеммер
* soundex - заменить ключевые слова их кодом SOUNDEX
* metaphone - заменить ключевые слова их кодом METAPHONE
* icu_chinese - применить сегментацию китайского текста с использованием ICU
* jieba_chinese - применить сегментацию китайского текста с использованием Jieba (требует пакет `manticore-language-packs`)
* libstemmer_* . Подробности смотрите в [списке поддерживаемых языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md)

Можно указать несколько стеммеров, разделенных запятыми. Они будут применяться к входящим словам в указанном порядке, и обработка остановится, как только один из стеммеров изменит слово. Кроме того, когда включена функция [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms), слово сначала ищется в словаре словоформ. Если в словаре есть соответствующая запись, стеммеры вообще не применяются. [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) можно использовать для реализации исключений стемминга.

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

Список полей, для которых пропускается морфологическая предобработка. Необязательный параметр, по умолчанию пустой (применять предобработчики ко всем полям).

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

Минимальная длина слова, при которой включается стемминг. Необязательный параметр, по умолчанию равен 1 (стемминг для всех слов).

Стеммеры не идеальны и иногда могут давать нежелательные результаты. Например, пропуск ключевого слова "gps" через стеммер Портера для английского языка даёт "gp", что не совсем соответствует цели. Функция `min_stemming_len` позволяет отключать стемминг в зависимости от длины исходного слова, т.е. избегать стемминга слишком коротких слов. Ключевые слова, которые короче заданного порога, стеммингу подвергаться не будут. Обратите внимание, что ключевые слова, длина которых в точности равна указанной, **будут** проходить стемминг. Таким образом, чтобы избежать стемминга для 3-символьных ключевых слов, следует указать значение 4. Для более тонкого контроля используйте функцию [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms).

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

Эта опция позволяет индексировать исходные ключевые слова вместе с их морфологически изменёнными версиями. Однако исходные ключевые слова, переопределённые с помощью [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) и [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md), не могут быть проиндексированы. Значение по умолчанию — 0, что означает, что функция отключена по умолчанию.

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

Включить или отключить HMM в инструменте сегментации Jieba. Необязательный параметр; по умолчанию равен 1.

В Jieba опция HMM (Hidden Markov Model, скрытая марковская модель) относится к алгоритму, используемому для сегментации слов. В частности, она позволяет Jieba выполнять китайскую сегментацию слов, распознавая неизвестные слова, особенно те, которых нет в её словаре.

Jieba в основном использует словарный метод для сегментации известных слов, но когда опция HMM включена, она применяет статистическую модель для определения вероятных границ слов для слов или фраз, отсутствующих в её словаре. Это особенно полезно для сегментации новых или редких слов, имён и сленга.

Вкратце, опция `jieba_hmm` помогает повысить точность сегментации ценой производительности индексации. Она должна использоваться вместе с `morphology = jieba_chinese`, см. [Китайский, японский и корейский (CJK) и тайский языки](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

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

Режим сегментации Jieba. Необязательный параметр; по умолчанию `accurate`.

В точном режиме (accurate) Jieba разбивает предложение на наиболее точные слова с использованием сопоставления по словарю. Этот режим ориентирован на точность, обеспечивая максимально возможную корректность сегментации.

В полном режиме (full) Jieba пытается разбить предложение на все возможные комбинации слов, стремясь включить все потенциальные слова. Этот режим ориентирован на максимизацию полноты охвата (recall), то есть он идентифицирует как можно больше слов, даже если некоторые из них перекрываются или используются реже. Он возвращает все слова, найденные в его словаре.

В режиме поиска Jieba разбивает текст на целые слова и более мелкие части, сочетая точную сегментацию с дополнительной детализацией, предоставляя перекрывающиеся фрагменты слов. Этот режим балансирует между точностью и полнотой, что делает его полезным для поисковых систем.

`jieba_mode` должен использоваться с `morphology = jieba_chinese`. Смотрите [Языки с непрерывными сценариями (Chinese, Japanese, Korean (CJK) и Thai)](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

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

Jieba, библиотека для сегментации китайского текста, использует файлы словарей для помощи в разделении слов. Формат этих файлов словаря следующий: каждую строку содержит слово, разделенное на три части пробелами — само слово, частотность слова и часть речи (POS tag). Частотность слова и POS tag опциональны и могут быть опущены. Файл словаря должен быть в кодировке UTF-8.

Пример:

```
创新办 3 i
云计算 5
凱特琳 nz
台中
```

`jieba_user_dict_path` должен использоваться с `morphology = jieba_chinese`. Для более подробной информации см. [Языки с непрерывными сценариями (Chinese, Japanese, Korean (CJK) и Thai)](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

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
CODE_BLOCK74
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
