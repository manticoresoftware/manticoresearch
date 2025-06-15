# Низкоуровневая токенизация

Когда текст индексируется в Manticore, он разбивается на слова, и выполняется преобразование регистра так, чтобы слова вроде "Abc", "ABC" и "abc" рассматривались как одинаковые.

Для правильного выполнения этих операций Manticore должен знать:
* кодировку исходного текста (которая всегда должна быть UTF-8)
* какие символы считаются буквами, а какие — нет
* какие буквы должны преобразовываться в другие буквы (case folding)

Вы можете настроить эти параметры для каждой таблицы отдельно с помощью опции [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). charset_table задаёт массив, который отображает буквенные символы в их версии с преобразованием регистра (или в любые другие символы, которые вы предпочитаете). Символы, отсутствующие в массиве, считаются не буквами и будут рассматриваться как разделители слов при индексации или поиске в этой таблице.

По умолчанию используется набор символов `non_cont`, который включает [большинство языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md).

Вы также можете определить правила замены текстовых шаблонов. Например, со следующими правилами:

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

Текст `RED TUBE 5" LONG` будет индексироваться как `COLOR TUBE 5 INCH LONG`, а `PLANK 2" x 4"` будет индексироваться как `PLANK 2 INCH x 4 INCH`. Эти правила применяются в указанном порядке. Правила также применяются к запросам, поэтому поиск `BLUE TUBE` фактически будет поиском `COLOR TUBE`.

Больше информации о [regexp_filter здесь](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter).

## Настройки конфигурации индекса

### charset_table

```ini
# default
charset_table = non_cont

# only English and Russian letters
charset_table = 0..9, A..Z->a..z, _, a..z, \
U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

# english charset defined with alias
charset_table = 0..9, english, _

# you can override character mappings by redefining them, e.g. for case insensitive search with German umlauts you can use:
charset_table = non_cont, U+00E4, U+00C4->U+00E4, U+00F6, U+00D6->U+00F6, U+00FC, U+00DC->U+00FC, U+00DF, U+1E9E->U+00DF
```

<!-- example charset_table -->
`charset_table` задаёт массив, который отображает буквенные символы в их версии с преобразованием регистра (или в любые другие символы, если вы предпочитаете). По умолчанию используется набор символов `non_cont`, который включает большинство языков с [непрерывными](https://en.wikipedia.org/wiki/Scriptio_continua) скриптами.

`charset_table` является основной частью процесса токенизации Manticore, который извлекает ключевые слова из текста документа или текста запроса. Он контролирует, какие символы считаются допустимыми и как они должны преобразовываться (например, удаляется ли различие регистра или нет).

По умолчанию каждый символ отображается в 0, что значит, что он не считается допустимым ключевым словом и рассматривается как разделитель. Как только символ упоминается в таблице, он отображается в другой символ (чаще всего в себя самого или в строчную букву) и рассматривается как часть допустимого ключевого слова.

charset_table использует список отображений, разделённых запятыми, чтобы объявить символы как допустимые или отобразить их в другие символы. Есть сокращения для отображения диапазонов символов сразу:

* Отображение одного символа: `A->a`. Объявляет исходный символ 'A' как допустимый в ключевых словах и отображает его в символ назначения 'a' (но не объявляет 'a' как допустимый).
* Отображение диапазона: `A..Z->a..z`. Объявляет все символы в исходном диапазоне как допустимые и отображает их в символы назначения. Не объявляет диапазон назначения как допустимый. Проверяет длины обоих диапазонов.
* Отображение одиночного символа: `a`. Объявляет символ как допустимый и отображает его в самого себя. Эквивалентно отображению одного символа `a->a`.
* Отображение одиночного диапазона: `a..z`. Объявляет все символы в диапазоне как допустимые и отображает их в самих себя. Эквивалентно отображению диапазона `a..z->a..z`.
* Отображение с шагом через один: `A..Z/2`. Отображает каждую пару символов во второй символ. Например, `A..Z/2` эквивалентен `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`. Это сокращение полезно для Unicode-блоков, где прописные и строчные буквы идут в перемешку.

Для символов с кодами от 0 до 32 и символов в диапазоне от 127 до 8-битного ASCII и Unicode, Manticore всегда рассматривает их как разделители. Чтобы избежать проблем с кодировкой в конфигурационных файлах, 8-битные ASCII символы и Unicode символы должны указываться в форме `U+XXX`, где `XXX` — шестнадцатеричный номер кода точки. Минимально допустимый код Unicode символа — `U+0021`.

Если стандартных отображений недостаточно для ваших нужд, вы можете переопределить отображения символов, указав их снова с другим отображением. Например, если встроенный массив `non_cont` включает символы `Ä` и `ä` и отображает их обоих в ASCII символ `a`, вы можете переопределить эти символы, добавив коды Unicode для них, так:

```
charset_table = non_cont,U+00E4,U+00C4
```

для чувствительного к регистру поиска или

```
charset_table = non_cont,U+00E4,U+00C4->U+00E4
```

для нечувствительного к регистру поиска.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'charset_table' => '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  charset_table = 0..9, A..Z->a..z, _, a..z, \
    U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example charset_table 2 -->
Помимо определения символов и отображений, существуют несколько встроенных алиасов, которые можно использовать. Текущие алиасы:
* `chinese`
* `cjk`
* `cont`
* `english`
* `japanese`
* `korean`
* `non_cont` (`non_cjk`)
* `russian`
* `thai`

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'charset_table' => '0..9, english, _'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  charset_table = 0..9, english, _

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

Если вы хотите поддерживать разные языки в вашем поиске, может быть утомительно задавать наборы допустимых символов и правила схлопывания для всех языков. Мы упростили эту задачу для вас, предоставив стандартные таблицы наборов символов, `non_cont` и `cont`, которые охватывают языки с раздельным и непрерывным (китайский, японский, корейский, тайский) письмом соответственно. В большинстве случаев этих наборов символов будет достаточно для ваших нужд.

Обратите внимание, что следующие языки в настоящее время **не** поддерживаются:
* Ассамский
* Бишнуприя
* Бухид
* Гаро
* Хмонг
* Хо
* Коми
* Мяо с крупными цветами
* Маба
* Майтхили
* Марати
* Менде
* Мру
* Миене
* Нгамбай
* Ория
* Сантали
* Синдхи
* Силхети

Все остальные языки, перечисленные в [Unicode languages
list](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/), поддерживаются по умолчанию.

<!-- example charset_table 3 -->
Чтобы работать как с cont, так и с non-cont языками, задайте параметры в вашем конфигурационном файле, как показано ниже (с [исключением](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md) для китайского):

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'"
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
  charset_table       = non_cont
  ngram_len           = 1
  ngram_chars         = cont

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

Если вам не требуется поддержка языков с непрерывным письмом, вы можете просто не указывать параметры [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) и [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars).
Для получения дополнительной информации по этим параметрам обратитесь к соответствующим разделам документации.

Для отображения одного символа в несколько символов или наоборот, вы можете использовать [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter).

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
Список смешиваемых символов. Необязательный параметр, по умолчанию пустой.

Смешиваемые символы индексируются как разделители и как допустимые символы. Например, когда `&` определён как смешиваемый символ, а `AT&T` встречается в индексируемом документе, будут проиндексированы три различных ключевых слова: `at&t`, `at` и `t`.

Кроме того, смешиваемые символы могут влиять на индексацию таким образом, что ключевые слова индексируются так, как если бы смешиваемых символов вообще не было. Это особенно заметно, когда указан параметр `blend_mode = trim_all`. Например, фраза `some_thing` будет проиндексирована как `some`, `something` и `thing` с `blend_mode = trim_all`.

Следует быть осторожным при использовании смешиваемых символов, поскольку определение символа как смешиваемого означает, что он больше не является разделителем.
* Поэтому если вы добавите запятую в `blend_chars` и выполните поиск по `dog,cat`, это будет рассматриваться как единый токен `dog,cat`. Если `dog,cat` **не** был проиндексирован как `dog,cat`, но остался только как `dog cat`, то совпадения не будет.
* Следовательно, этим поведением следует управлять с помощью настройки [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode).

Позиции для токенов, полученных путём замены смешиваемых символов на пробел, задаются как обычно, и обычные ключевые слова будут индексироваться, как если бы не было задано никаких `blend_chars`. Дополнительный токен, который объединяет смешиваемые и не смешиваемые символы, будет помещаться в начальную позицию. Например, если `AT&T company` встречается в самом начале текстового поля, `at` получит позицию 1, `t` — позицию 2, `company` — позицию 3, а `AT&T` также будет иметь позицию 1, смешиваясь с начальным обычным ключевым словом. В результате запросы по `AT&T` или просто `AT` будут находить этот документ. Также будет найдено совпадение по фразе `"AT T"`, а также по фразе `"AT&T company"`.

Смешиваемые символы могут пересекаться с особыми символами, используемыми в синтаксисе запросов, например, в `T-Mobile` или `@twitter`. Где возможно, парсер запроса будет обрабатывать смешиваемый символ как смешиваемый. Например, если `hello @twitter` находится в кавычках (оператор фразы), парсер запроса обработает символ `@` как смешиваемый. Однако, если символ `@` не в кавычках, он будет считаться оператором. Поэтому рекомендуется экранировать ключевые слова.

Смешиваемые символы могут переназначаться так, чтобы несколько разных смешиваемых символов нормализовались в одну базовую форму. Это полезно при индексации нескольких альтернативных Unicode-кодов с эквивалентными глифами.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_chars' => '+, &, U+23, @->_'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  blend_chars = +, &, U+23, @->_

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### blend_mode

```ini
blend_mode = option [, option [, ...]]
option = trim_none | trim_head | trim_tail | trim_both | trim_all | skip_pure
```

<!-- example blend_mode -->
Режим индексации смешанных токенов включается директивой blend_mode.

По умолчанию токены, которые смешивают смешиваемые и не смешиваемые символы, индексируются полностью. Например, если и символ "@" и восклицательный знак находятся в `blend_chars`, строка `@dude!` будет проиндексирована как два токена: `@dude!` (со всеми смешиваемыми символами) и `dude` (без них). В результате, запрос `@dude` **не** найдёт это.

Параметр `blend_mode` добавляет гибкости в это поведение индексации. Он принимает список опций, разделённых запятыми, каждая из которых указывает вариант индексации токена.

Если указано несколько опций, будет проиндексировано несколько вариантов одного и того же токена. Обычные ключевые слова (полученные из этого токена заменой смешанных символов на разделитель) всегда индексируются.

Опции:

* `trim_none` - Индексировать весь токен
* `trim_head` - Обрезать начальные смешанные символы и индексировать получившийся токен
* `trim_tail` - Обрезать конечные смешанные символы и индексировать получившийся токен
* `trim_both` - Обрезать как начальные, так и конечные смешанные символы и индексировать получившийся токен
* `trim_all` - Обрезать начальные, конечные и средние смешанные символы и индексировать получившийся токен
* `skip_pure` - Не индексировать токен, если он состоит только из смешанных символов

Используя `blend_mode` с примером строки `@dude!` выше, настройка `blend_mode = trim_head, trim_tail` приведёт к индексации двух токенов: `@dude` и `dude!`. Использование `trim_both` не будет иметь эффекта, так как обрезка обоих смешанных символов даст `dude`, который уже индексируется как обычное ключевое слово. Индексация `@U.S.A.` с `trim_both` (если предположить, что точка - смешанный символ) приведёт к индексации `U.S.A`. Наконец, `skip_pure` позволяет игнорировать последовательности, состоящие только из смешанных символов. Например, строка `one @@@ two` будет индексирована как `one two` и совпадёт с этой фразой. По умолчанию такого нет, потому что полностью смешанный токен индексируется и смещает позицию второго ключевого слова.

Поведение по умолчанию — индексировать весь токен, эквивалентно `blend_mode = trim_none`.

Учтите, что использование режимов смешивания ограничивает ваш поиск, даже в режиме по умолчанию `trim_none`, если считать `.` смешанным символом:
* `.dog.` при индексации станет `.dog. dog`
* и вы не сможете найти его по запросу `dog.`.

Использование большего количества режимов повышает вероятность совпадения вашего ключевого слова.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_mode' => 'trim_tail, skip_pure',
            'blend_chars' => '+, &'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  blend_mode = trim_tail, skip_pure
  blend_chars = +, &

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### min_word_len

```ini
min_word_len = length
```

<!-- example min_word_len -->

min_word_len — необязательный параметр конфигурации индекса в Manticore, задающий минимальную длину индексируемого слова. Значение по умолчанию — 1, что означает индексацию всех слов.

Индексируются только те слова, длина которых не короче этого минимума. Например, если min_word_len равен 4, слово 'the' не будет индексироваться, а 'they' — будет.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_word_len = '4'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) min_word_len = '4'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'min_word_len' => '4'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) min_word_len = '4'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  min_word_len = 4

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ngram_len

```ini
ngram_len = 1
```

<!-- example ngram_len -->
Длина N-грамм для индексирования N-граммами. Необязательный параметр, по умолчанию 0 (индексация N-грамм отключена). Известные значения — 0 и 1.

N-граммы обеспечивают базовую поддержку языков с непрерывным письмом в текстах без сегментации. Проблема поиска в языках с непрерывным письмом — отсутствие явных разделителей между словами. В некоторых случаях может не подойти сегментация на основе словаря, например, [для китайского языка](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md). В таких случаях сегментация на N-граммы тоже может работать хорошо.

Когда функция включена, потоки таких языков (или любые другие символы, определённые в [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)) индексируются как N-граммы. Например, если входной текст — "ABCDEF" (где A-F — символы языка), а ngram_len=1, то он будет индексироваться как "A B C D E F". На данный момент поддерживается только ngram_len=1. Только символы, перечисленные в таблице [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars), будут разбиты таким образом; остальные не изменятся.

Обратите внимание, что если поисковый запрос сегментирован, то есть между отдельными словами есть разделители, то заключение слов в кавычки и использование расширенного режима позволит найти правильные совпадения даже если текст был **не** сегментирован. Например, исходный запрос `BC DEF` после оборачивания в кавычки на стороне приложения будет выглядеть как `"BC" "DEF"` (*с* кавычками). Этот запрос будет передан в Manticore и внутри тоже разбит на 1-граммы, в результате получится запрос `"B C" "D E F"`, всё ещё с кавычками — оператором фразового совпадения. И он найдёт совпадение с текстом, даже если в тексте не было разделителей.

Даже если поисковой запрос не сегментирован, Manticore всё равно должен давать хорошие результаты благодаря ранжированию на основе фраз: более близкие фразовые совпадения (в случае слов из N-грамм могут означать более близкие многосимвольные совпадения) будут вверху списка.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = cont
  ngram_len = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ngram_chars

```ini
ngram_chars = cont

ngram_chars = cont, U+3000..U+2FA1F
```

<!-- example ngram_chars -->
Список символов N-грамм. Необязательно, по умолчанию пустой.

Используется вместе с параметром [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len), этот список определяет символы, последовательности которых подлежат извлечению N-грамм. Слова, состоящие из других символов, не будут затронуты функцией индексации N-грамм. Формат значения идентичен [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Символы N-грамм не могут присутствовать в [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'U+3000..U+2FA1F',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = U+3000..U+2FA1F
  ngram_len = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```

<!-- end -->

<!-- example ngram_chars 2 -->
Также можно использовать псевдоним для нашей стандартной таблицы N-грамм, как показано в примере. В большинстве случаев этого будет достаточно.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = cont
  ngram_len = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ignore_chars

```ini
ignore_chars = U+AD
```

<!-- example ignore_chars -->
Список игнорируемых символов. Необязательно, по умолчанию пустой.

Полезно в случаях, когда некоторые символы, такие как мягкий перенос (U+00AD), должны не просто рассматриваться как разделители, а полностью игнорироваться. Например, если '-' просто отсутствует в charset_table, текст "abc-def" будет индексироваться как ключевые слова "abc" и "def". Напротив, если '-' добавлен в список ignore_chars, тот же текст будет индексироваться как одно ключевое слово "abcdef".

Синтаксис такой же, как для [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), но разрешено объявлять только символы, запрещено делать отображения для них. Кроме того, игнорируемые символы не должны присутствовать в charset_table.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'ignore_chars' => 'U+AD'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ignore_chars = U+AD

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### bigram_index

```ini
bigram_index = {none|all|first_freq|both_freq}
```

<!-- example bigram_index -->
Режим индексации биграмм. Необязательно, по умолчанию отсутствует.

Индексация биграмм — это функция для ускорения поиска фраз. При индексации сохраняется список документов для всех или некоторых пар соседних слов в индексе. Такой список затем может использоваться во время поиска для значительного ускорения сопоставления фразы или подфразы.

`bigram_index` управляет выбором конкретных пар слов. Известные режимы:

* `all` — индексировать каждую пару слов
* `first_freq` — индексировать только пары слов, где *первое* слово в списке часто встречающихся слов (см. [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)). Например, с `bigram_freq_words = the, in, i, a`, при индексации текста "alone in the dark" будут сохранены пары "in the" и "the dark" как биграммы, потому что они начинаются с частого слова (либо "in", либо "the"), а "alone in" **не будет** индексироваться, поскольку "in" является *вторым* словом в этой паре.
* `both_freq` — индексировать только пары слов, где оба слова частые. Продолжая тот же пример, в этом режиме индексация "alone in the dark" сохранит только "in the" (самый бесполезный для поиска из всех) как биграмму, но ни одна из других пар слов не будет сохранена.

Для большинства случаев режим `both_freq` будет лучшим, но результаты могут варьироваться.

Важно отметить, что `bigram_index` работает только на уровне токенизации и не учитывает такие преобразования, как `morphology`, `wordforms` или `stopwords`. Это означает, что создаваемые им токены очень простые, что делает поиск фраз более точным и строгим. Хотя это может улучшить точность сопоставления фраз, это также снижает способность системы распознавать разные формы слов или варианты их написания.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'both_freq'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  bigram_index = both_freq
  bigram_freq_words = the, a, you, i

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### bigram_freq_words

```ini
bigram_freq_words = the, a, you, i
```

<!-- example bigram_freq_words -->
Список ключевых слов, считающихся «частыми» при индексировании биграмм. Необязательно, по умолчанию пустой.

Некоторые режимы индексации биграмм (см. [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)) требуют определить список частотных ключевых слов. Их **не** следует путать со стоп-словами. Стоп-слова полностью исключаются при индексации и поиске. Частотные ключевые слова используются биграммами только для определения, следует ли индексировать текущую пару слов или нет.

`bigram_freq_words` позволяет определить список таких ключевых слов.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'first_freq'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  bigram_freq_words = the, a, you, i
  bigram_index = first_freq

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### dict

```ini
dict = {keywords|crc}
```

<!-- example dict -->
Тип словаря ключевых слов определяется одним из двух известных значений – «crc» или «keywords». Это необязательно, по умолчанию используется «keywords».

Использование режима словаря ключевых слов (dict=keywords) может значительно уменьшить нагрузку при индексации и обеспечить поиск по подстрокам в больших коллекциях. Этот режим может использоваться как для обычных, так и для RT таблиц.

CRC-словари не хранят исходный текст ключевого слова в индексе. Вместо этого они заменяют ключевые слова контрольной суммой (вычисленной с использованием FNV64) как при поиске, так и при индексации. Это значение используется внутренне в индексе. Такой подход имеет два недостатка:
* Во-первых, существует риск столкновений контрольных сумм между разными парами ключевых слов. Этот риск увеличивается пропорционально количеству уникальных ключевых слов в индексе. Тем не менее, эта проблема незначительна, поскольку вероятность одного столкновения FNV64 в словаре из 1 миллиарда записей примерно 1 из 16, или 6.25 процента. Большинство словарей будут иметь значительно меньше миллиарда ключевых слов, учитывая, что типичный разговорный язык содержит от 1 до 10 миллионов форм слов.
* Во-вторых, и более важно, поиск подстрок с контрольными суммами не так прост. Manticore решил эту проблему предварительной индексацией всех возможных подстрок как отдельных ключевых слов (см. директивы [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len), [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)). Этот метод имеет дополнительное преимущество — максимально быстрый поиск подстрок. Но предварительная индексация всех подстрок значительно увеличивает размер индекса (часто в 3-10 раз и более) и соответственно влияет на время индексации, делая поиск подстрок по большим индексам менее практичным.

Словарь ключевых слов решает обе эти проблемы. Он хранит ключевые слова в индексе и выполняет расширение подстановочных знаков во время поиска. Например, поиск по префиксу `test*` может внутренне расшириться в запрос `test|tests|testing` на основе содержимого словаря. Этот процесс расширения полностью невидим для приложения, за исключением того, что отдельно отчитывается статистика по каждому из всех найденных ключевых слов.

Для поиска по подстрокам (инфикс) могут использоваться расширенные подстановочные знаки. Специальные символы, такие как `?` и `%`, совместимы с поиском по подстрокам (инфикс) (например, `t?st*`, `run%`, `*abc*`). Обратите внимание, что [операторы подстановочных знаков](../../Searching/Full_text_matching/Operators.md#Wildcard-operators) и [оператор REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) работают только с параметром `dict=keywords`.

Индексация с использованием словаря ключевых слов примерно на 1.1x–1.3x медленнее, чем обычная индексация без подстрок, но значительно быстрее, чем индексация с поддержкой подстрок (префиксных или инфиксных). Размер индекса будет лишь немного больше, чем у стандартной таблицы без поддержки подстрок, с общей разницей в 1..10%. Время обычного поиска по ключевым словам должно быть практически таким же или идентичным для всех трех типов индексов (CRC без подстрок, CRC с подстроками, keywords). Время поиска по подстрокам может значительно варьироваться в зависимости от количества ключевых слов, совпадающих с данной подстрокой (то есть, во сколько ключевых слов расширяется поисковый запрос). Максимальное количество совпадающих ключевых слов ограничено директивой [expansion_limit](../../Server_settings/Searchd.md#expansion_limit).

В итоге, словари keywords и CRC предлагают два различных компромисса для поиска по подстрокам. Вы можете пожертвовать временем индексации и размером индекса, чтобы достичь максимально быстрого поиска в худшем случае (словарь CRC) или минимально повлиять на время индексации, но «пожертвовать» временем поиска в худшем случае, когда префикс расширяется в большое количество ключевых слов (словарь keywords).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) dict = 'keywords'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) dict = 'keywords'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'dict' => 'keywords'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) dict = 'keywords'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  dict = keywords

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### embedded_limit

```ini
embedded_limit = size
```

<!-- example embedded_limit -->
Лимит размера встроенных исключений, форм слов или файла стоп-слов. Опционально, по умолчанию 16K.

При создании таблицы упомянутые выше файлы могут сохраняться либо отдельно вместе с таблицей, либо быть встроенными непосредственно в таблицу. Файлы размером меньше `embedded_limit` сохраняются в таблице. Для файлов большего размера сохраняются только имена файлов. Это также упрощает перенос файлов таблицы на другой компьютер; можно обойтись копированием всего одного файла.

С меньшими файлами такое внедрение уменьшает количество внешних файлов, от которых зависит таблица, и облегчает поддержку. Но в то же время нет смысла внедрять словарь wordforms размером 100 МБ в крошечную дельта-таблицу. Поэтому необходим порог по размеру, и `embedded_limit` является этим порогом.

<!-- request CONFIG -->

```ini
table products {
  embedded_limit = 32K

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### global_idf

```ini
global_idf = /path/to/global.idf
```

<!-- example global_idf -->
Путь к файлу с глобальными (на уровне кластера) ключевыми IDF. Опционально, по умолчанию пусто (использовать локальные IDF).

В многотабличном кластере частоты ключевых слов, скорее всего, будут отличаться между разными таблицами. Это значит, что когда функция ранжирования использует значения на основе TF-IDF, например факторы семейства BM25, результаты могут быть ранжированы немного по-разному в зависимости от узла кластера, на котором они находятся.

Самый простой способ исправить эту проблему — создать и использовать глобальный частотный словарь, или коротко глобальный IDF-файл. Эта директива позволяет указать расположение этого файла. Рекомендуется (но не обязательно) использовать расширение .idf. Когда IDF-файл задан для конкретной таблицы *и* [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) установлен в 1, движок будет использовать частоты ключевых слов и счётчики документов из глобального файла global_idf, а не только из локальной таблицы. Таким образом, IDF и значения, зависящие от них, будут оставаться согласованными по всему кластеру.

IDF-файлы могут использоваться несколькими таблицами. Только одна копия IDF-файла будет загружена `searchd`, даже если многие таблицы ссылаются на этот файл. Если содержимое IDF-файла изменится, новые данные могут быть загружены с помощью SIGHUP.

Вы можете создать .idf файл с помощью утилиты [indextool](../../Miscellaneous_tools.md#indextool), сначала выгрузив словари с помощью переключателя `--dumpdict dict.txt --stats`, затем конвертировав их в формат .idf с помощью `--buildidf`, а затем объединив все .idf файлы по кластеру с помощью `--mergeidf`.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'global_idf' => '/usr/local/manticore/var/global.idf'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  global_idf = /usr/local/manticore/var/global.idf

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### hitless_words

```ini
hitless_words = {all|path/to/file}
```

<!-- example hitless_words -->
Список hitless-слов. Опционально, допустимые значения: 'all' или имя файла со списком.

По умолчанию полнотекстовый индекс Manticore хранит не только список документов, в которых встречается каждое ключевое слово, но и список позиций ключевого слова в документе (известный как hitlist). Hitlist позволяют выполнять поиск по фразам, близости, строгому порядку и другим расширенным типам поиска, а также ранжирование по близости фраз. Однако hitlist для определённых часто встречающихся ключевых слов (которые по какой-то причине не могут быть остановлены, несмотря на частоту) может стать очень большим и замедлять обработку при запросах. Кроме того, в некоторых случаях нам может быть важен только булевый поиск по ключевым словам, без необходимости использовать операторы позиций (например, поиск по фразам) или ранжирование фраз.

Опция `hitless_words` позволяет создавать индексы, которые либо совсем не содержат позиционной информации (hitlist), либо пропускают её для определённых ключевых слов.

Hitless индекс обычно занимает меньше места, чем соответствующий обычный полнотекстовый индекс (примерно в 1.5 раза меньше). И индексация, и поиск будут быстрее, за счёт отсутствия поддержки позиционных запросов и позиционного ранжирования.

Если hitless-слова используются в позиционных запросах (например, в запросах по фразам), они исключаются из учитываемых позиций и используются как операнды без позиции. Например, если "hello" и "world" являются hitless словами, а "simon" и "says" нет, то фразовый запрос `"simon says hello world"` будет преобразован в `("simon says" & hello & world)`, где "hello" и "world" могут встретиться где угодно в документе, а "simon says" — как точная фраза.

Позиционный запрос, содержащий только hitless-слова, приведёт к пустому узлу фразы, и, следовательно, весь запрос вернёт пустой результат и предупреждение. Если весь словарь сделан hitless (с помощью `all`), то на таком индексе можно использовать только булевое совпадение.



<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) hitless_words = 'all'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) hitless_words = 'all'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'hitless_words' => 'all'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  hitless_words = all

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### index_field_lengths

```ini
index_field_lengths = {0|1}
```

<!-- example index_field_lengths -->
Включает вычисление и сохранение длины поля (как для каждого документа, так и усреднённые по индексу значения) в полнотекстовом индексе. Опционально, по умолчанию 0 (не вычислять и не сохранять).

Когда `index_field_lengths` установлен в 1, Manticore будет:
* создавать соответствующий атрибут длины для каждого полнотекстового поля, с тем же именем, но с суффиксом `__len`
* вычислять длину поля (в количестве ключевых слов) для каждого документа и сохранять в соответствующий атрибут
* вычислять средние значения по индексу. Атрибуты длины будут иметь специальный тип TOKENCOUNT, но их значения фактически являются обычными 32-битными целыми числами и доступны в общем доступе.

[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) и [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) функции в ранжировщике выражений основаны на этих длинах и требуют включения `index_field_lengths`. Исторически Manticore использовал упрощённый, урезанный вариант BM25, который, в отличие от полной функции, **не** учитывал длину документа. Также поддерживается как полный вариант BM25, так и его расширение для нескольких полей, называемое BM25F. Они требуют длину на документ и длину по полю соответственно. Отсюда и дополнительная директива.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_field_lengths = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) index_field_lengths = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_field_lengths' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  index_field_lengths = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### index_token_filter

```ini
index_token_filter = my_lib.so:custom_blend:chars=@#&
```

<!-- example index_token_filter -->
Фильтр токенов во время индексации для полнотекстового индексирования. Необязательно, значение по умолчанию - пусто.

Директива index_token_filter задаёт необязательный фильтр токенов во время индексации для полнотекстового индексирования. Эта директива используется для создания пользовательского токенизатора, который формирует токены согласно пользовательским правилам. Фильтр создаётся индексатором при индексации исходных данных в простую таблицу или RT-таблицей при обработке операторов `INSERT` или `REPLACE`. Плагины определяются в формате `имя библиотеки:имя плагина:необязательная строка настроек`. Например, `my_lib.so:custom_blend:chars=@#&`.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_token_filter' => 'my_lib.so:custom_blend:chars=@#&'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  index_token_filter = my_lib.so:custom_blend:chars=@#&

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### overshort_step

```ini
overshort_step = {0|1}
```

<!-- example overshort_step -->
Инкремент позиции для слишком коротких (меньше [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)) ключевых слов. Необязательно, допустимые значения 0 и 1, значение по умолчанию 1.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) overshort_step = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) overshort_step = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'overshort_step' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) overshort_step = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  overshort_step = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### phrase_boundary

```ini
phrase_boundary = ., ?, !, U+2026 # horizontal ellipsis
```

<!-- example phrase_boundary -->
Список символов-разделителей фраз. Необязательно, значение по умолчанию пустое.

Этот список контролирует, какие символы будут рассматриваться как границы фразы, чтобы корректировать позиции слов и включать эмуляцию поиска на уровне фразы через поиск по близости. Синтаксис похож на [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), но отображения не допускаются, а символы-разделители не должны пересекаться с чем-либо другим.

На границе фразы будет добавлено дополнительное смещение позиции слова, заданное директивой [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step). Это позволяет реализовать поиск на уровне фраз через запросы близости: слова в разных фразах гарантированно будут находиться на расстоянии более phrase_boundary_step друг от друга; таким образом, поиск по близости в пределах этого расстояния будет эквивалентен поиску на уровне фразы.

Условие границы фразы возникает тогда и только тогда, когда после такого символа следует разделитель; это нужно, чтобы избежать восприятия сокращений, таких как S.T.A.L.K.E.R, или URL-адресов, как нескольких фраз.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary' => '., ?, !, U+2026',
             'phrase_boundary_step' => '10'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')
```

<!-- intro -->
##### Pytho-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  phrase_boundary = ., ?, !, U+2026
  phrase_boundary_step = 10

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### phrase_boundary_step

```ini
phrase_boundary_step = 100
```

<!-- example phrase_boundary_step -->
Инкремент позиции слова на границе фразы. Необязательно, значение по умолчанию 0.

При достижении границы фразы текущая позиция слова дополнительно увеличивается на это значение.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary_step' => '100',
             'phrase_boundary' => '., ?, !, U+2026'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  phrase_boundary_step = 100
  phrase_boundary = ., ?, !, U+2026

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### regexp_filter

```ini
# index '13"' as '13inch'
regexp_filter = \b(\d+)\" => \1inch

# index 'blue' or 'red' as 'color'
regexp_filter = (blue|red) => color
```

<!-- example regexp_filter -->
Регулярные выражения (regexps) используются для фильтрации полей и запросов. Эта директива является необязательной, может принимать несколько значений, и по умолчанию представляет собой пустой список регулярных выражений. Движок регулярных выражений, используемый в Manticore Search, — это RE2 от Google, который известен своей скоростью и безопасностью. Для подробной информации о синтаксисе, поддерживаемом RE2, вы можете посетить [руководство по синтаксису RE2](https://github.com/google/re2/wiki/Syntax).

В некоторых приложениях, таких как поиск товаров, может быть много способов обозначить продукт, модель или свойство. Например, `iPhone 3gs` и `iPhone 3 gs` (или даже `iPhone3 gs`) с большой вероятностью относятся к одному и тому же товару. Другой пример — различные варианты указания размера экрана ноутбука, такие как `13-inch`, `13 inch`, `13"`, или `13in`.

Регулярные выражения предоставляют механизм для задания правил, предназначенных для обработки таких случаев. В первом примере вы, возможно, сможете использовать файл wordforms для нескольких моделей iPhone, но во втором примере лучше указать правила, которые нормализуют "13-inch" и "13in" в нечто одинаковое.

Регулярные выражения, перечисленные в `regexp_filter`, применяются в том порядке, в каком они перечислены, на наиболее раннем этапе, ещё до любой другой обработки (включая [исключения](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)), даже до токенизации. То есть, regexps применяются к исходным полям при индексации и к исходному тексту поискового запроса при поиске.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'regexp_filter' => '(blue|red) => color'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  # index '13"' as '13inch'
  regexp_filter = \b(\d+)\" => \1inch

  # index 'blue' or 'red' as 'color'
  regexp_filter = (blue|red) => color

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->

