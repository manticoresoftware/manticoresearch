# Расширенная морфология

Морфологические препроцессоры могут применяться к словам во время индексации для нормализации различных форм одного и того же слова и улучшения сегментации. Например, английский стеммер может нормализовать "dogs" и "dog" в "dog", что приводит к одинаковым результатам поиска для обоих ключевых слов.

Manticore имеет четыре встроенных морфологических препроцессора:

*   **Лемматизатор**: сокращает слово до его корня или леммы. Например, "running" может быть сокращено до "run", а "octopi" может быть сокращено до "octopus". Обратите внимание, что некоторые слова могут иметь несколько соответствующих корневых форм. Например, "dove" может быть как прошедшей формой "dive", так и существительным, означающим птицу, как в "A white dove flew over the cuckoo's nest." В этом случае лемматизатор может сгенерировать все возможные корневые формы.
*   **Стеммер**: сокращает слово до его основы, удаляя или заменяя определенные известные суффиксы. Получившаяся основа может не быть действительным словом. Например, английский стеммер Портера сокращает "running" до "run", "business" до "busi" (не действительное слово) и вообще не уменьшает "octopi".
*   **Фонетические алгоритмы**: заменяют слова фонетическими кодами, которые одинаковы, даже если слова разные, но фонетически близкие.
*   **Алгоритмы разбиения слов**: разделяют текст на слова. В настоящее время доступны только для китайского языка.

## морфология

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->
Директива морфологии задает список морфологических препроцессоров, которые необходимо применить к индексируемым словам. Это необязательная настройка, по умолчанию не применяется никаких препроцессоров.

Manticore поставляется с встроенными морфологическими препроцессорами для:

* Лемматизаторов для английского, русского и немецкого языков
* Стеммеров для английского, русского, арабского и чешского языков
* Фонетических алгоритмов SoundEx и MetaPhone
* Алгоритма разбиения слов для китайского языка
* Стеммеров Snowball (libstemmer) для более чем [15 других языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) также доступны.

Лемматизаторы требуют словарные файлы `.pak`, которые можно установить с помощью пакетов `manticore-language-packs` или [скачать с сайта Manticore](https://manticoresearch.com/install/#other-downloads). В последнем случае словари необходимо поместить в директорию, указанную в [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base). 

Кроме того, настройка [lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) может быть использована для ускорения лемматизации, затрачивая больше ОЗУ на не сжатый кэш словаря.

Сегментация китайского языка может быть выполнена с использованием [ICU](http://site.icu-project.org/) или [Jieba](https://github.com/yanyiwu/cppjieba) (требует пакет `manticore-language-packs`). Оба библиотеки обеспечивают более точную сегментацию, чем n-граммы, но немного медленнее. Таблица [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) должна включать все китайские символы, что можно сделать с помощью наборов символов `cont`, `cjk` или `chinese`. Когда вы устанавливаете `morphology=icu_chinese` или `morphology=jieba_chinese`, документы сначала обрабатываются ICU или Jieba. Затем токенизатор обрабатывает результат согласно charset_table, и, наконец, применяются другие морфологические процессоры из опции `morphology`. Только те части текста, которые содержат китайский язык, передаются в ICU/Jieba для сегментации, в то время как другие части могут быть изменены различными способами, такими как разные морфологии или `charset_table`.

Встроенные стеммеры для английского и русского языков быстрее, чем их аналоги libstemmer, но могут давать немного разные результаты.

Реализация Soundex соответствует реализации MySQL. Реализация Metaphone основана на алгоритме Double Metaphone и индексирует основной код.

Чтобы использовать опцию `morphology`, укажите один или несколько встроенных вариантов, включая:
* none: не выполнять никакой морфологической обработки
* lemmatize_ru - применить русский лемматизатор и выбрать одну корневую форму
* lemmatize_uk - применить украинский лемматизатор и выбрать одну корневую форму (установите его сначала в [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) или [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer)). Для корректной работы лемматизатора убедитесь, что специфические украинские символы сохранены в вашей `charset_table`, так как по умолчанию они не сохраняются. Для этого переопределите их, например так: `charset_table='non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`. [Здесь](https://play.manticoresearch.com/ua-lemmatizer/) находится интерактивный курс о том, как установить и использовать украинский лемматизатор.
* lemmatize_en - применить английский лемматизатор и выбрать одну корневую форму
* lemmatize_de - применить немецкий лемматизатор и выбрать одну корневую форму
* lemmatize_ru_all - применить русский лемматизатор и индексировать все возможные корневые формы
* lemmatize_uk_all - применить украинский лемматизатор и индексировать все возможные корневые формы. Смотрите ссылки на установку выше и позаботьтесь о `charset_table`.
* lemmatize_en_all - применить английский лемматизатор и индексировать все возможные корневые формы
* lemmatize_de_all - применить немецкий лемматизатор и индексировать все возможные корневые формы
* stem_en - применить стеммер Портера для английского языка
* stem_ru - применить стеммер Портера для русского языка
* stem_enru - применить стеммеры Портера для английского и русского языков
* stem_cz - применить чешский стеммер
* stem_ar - применить арабский стеммер
* soundex - заменить ключевые слова их SOUNDEX кодом
* metaphone - заменить ключевые слова их METAPHONE кодом
* icu_chinese - применить сегментацию текста на китайском языке, используя ICU
* jieba_chinese - применить сегментацию текста на китайском языке, используя Jieba (требует пакет `manticore-language-packs`)
* libstemmer_* . Смотрите [список поддерживаемых языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) для получения подробностей

Несколько стеммеров могут быть указаны, разделенные запятыми. Они будут применяться к входящим словам в порядке, в котором они перечислены, и обработка остановится, как только один из стеммеров изменит слово. Кроме того, когда функция [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) включена, слово сначала будет искать в словаре форм слов. Если в словаре есть соответствующая запись, стеммеры применяться не будут. Функция [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) может быть использована для реализации исключений в стемминге.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'stem_en, libstemmer_sv\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'");
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

Список полей, для которых следует пропустить предварительную обработку морфологии. Необязательный, по умолчанию пусто (применять предварительные обработчики ко всем полям).

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = \'name\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'");
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

Минимальная длина слова, при которой включается стемминг. Необязательный, по умолчанию 1 (стемить всё).

Стеммеры не идеальны и могут иногда давать нежелательные результаты. Например, пропуск ключевого слова "gps" через стеммер Портера для английского приводит к "gp", что не является истинным намерением. Функция `min_stemming_len` позволяет подавить стемминг на основе длины исходного слова, т.е. избежать стемминга слишком коротких слов. Ключевые слова, которые короче заданного порога, не будут истемлены. Обратите внимание, что ключевые слова, которые ровно такой длины, как указано, **будут** истемлены. Поэтому для того, чтобы избежать стемминга 3-символьных ключевых слов, вы должны указать 4 в качестве значения. Для более тонкого контроля обратитесь к функции [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms).

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
##### Javascript:
<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = \'4\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'");
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

Эта опция позволяет индексировать оригинальные ключевые слова вместе с их морфологически изменёнными версиями. Однако оригинальные ключевые слова, которые переназначаются с помощью [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) и [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md), не могут быть проиндексированы. Значение по умолчанию равно 0, что указывает на то, что эта функция отключена по умолчанию.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = \'1\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'");
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

Включить или отключить HMM в инструменте сегментации Jieba. Необязательный параметр; значение по умолчанию - 1.

В Jieba опция HMM (Скрытая модель Маркова) относится к алгоритму, используемому для сегментации слов. В частности, она позволяет Jieba выполнять сегментацию китайских слов, распознавая неизвестные слова, особенно те, которые отсутствуют в его словаре.

Jieba в основном использует метод на основе словаря для сегментации известных слов, но когда опция HMM включена, она применяет статистическую модель для определения вероятных границ слов для слов или фраз, которые не присутствуют в её словаре. Это особенно полезно для сегментации новых или редких слов, имен и жаргона.

В целом, опция `jieba_hmm` помогает повысить точность сегментации за счёт снижения производительности индексации. Она должна использоваться с `morphology = jieba_chinese`, см. [Китайские, японские и корейские (CJK) и тайский языки](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_hmm = \'0\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'");
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

Режим сегментации Jieba. Необязательно; по умолчанию `accurate`.

В режиме accurate Jieba делит предложение на наиболее точные слова с использованием словарного соответствия. Этот режим фокусируется на точности, обеспечивая, чтобы сегментация была как можно более точной.

В режиме full Jieba пытается разбить предложение на все возможные комбинации слов, стремясь включить все потенциальные слова. Этот режим ориентирован на максимизацию полноты, что означает, что он определяет как можно больше слов, даже если некоторые из них перекрываются или используются реже. Он возвращает все слова, найденные в его словаре.

В режиме search Jieba разбивает текст как на целые слова, так и на более мелкие части, комбинируя точную сегментацию с дополнительной деталью, предоставляя перекрывающиеся фрагменты слов. Этот режим сбалансирован между точностью и полнотой, что делает его полезным для поисковых систем.

`jieba_mode` следует использовать с `morphology = jieba_chinese`. См. [Китайские, японские, корейские (CJK) и тайские языки](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_mode = \'full\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'");
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

Путь к пользовательскому словарю Jieba. Необязательно.

Jieba, библиотека сегментации китайского текста, использует файлы словаря для помощи в сегментации слов. Формат этих словарных файлов выглядит следующим образом: каждая строка содержит слово, разделенное на три части, разделенные пробелами — само слово, частота слова и тег части речи (POS). Частота слова и тег части речи необязательны и могут быть опущены. Файл словаря должен быть закодирован в UTF-8.

Пример:

```
创新办 3 i
云计算 5
凱特琳 nz
台中
```

`jieba_user_dict_path` следует использовать с `morphology = jieba_chinese`. Для получения дополнительных сведений см. [Китайские, японские, корейские (CJK) и тайские языки](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_user_dict_path = \'/usr/local/manticore/data/user-dict.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'");
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

