# Низкоуровневая токенизация

Когда текст индексируется в Manticore, он разбивается на слова, и выполняется преобразование регистра, чтобы такие слова, как "Abc", "ABC" и "abc", рассматривались как одно и то же слово.

Для правильного выполнения этих операций Manticore должен знать:
* кодировку исходного текста (которая всегда должна быть UTF-8)
* какие символы считаются буквами, а какие нет
* какие буквы должны преобразовываться в другие буквы

Вы можете настроить эти параметры для каждой таблицы отдельно, используя опцию [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). charset_table задаёт массив, который отображает буквенные символы в их версии с приведённым регистром (или в любые другие символы по вашему выбору). Символы, отсутствующие в массиве, считаются не буквами и будут рассматриваться как разделители слов при индексировании или поиске в этой таблице.

По умолчанию используется набор символов `non_cont`, который включает [большинство языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md).

Вы также можете определить правила замены текстовых шаблонов. Например, с помощью следующих правил:

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

Текст `RED TUBE 5" LONG` будет индексироваться как `COLOR TUBE 5 INCH LONG`, а `PLANK 2" x 4"` будет индексироваться как `PLANK 2 INCH x 4 INCH`. Эти правила применяются в указанном порядке. Правила также применяются к запросам, поэтому поиск по `BLUE TUBE` фактически будет искать `COLOR TUBE`.

Подробнее о [regexp_filter можно узнать здесь](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter).

## Опции конфигурации индекса

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
`charset_table` задаёт массив, который отображает буквенные символы в их версии с приведённым регистром (или в любые другие символы, если вы предпочитаете). По умолчанию используется набор символов `non_cont`, который включает большинство языков с [непрерывными](https://en.wikipedia.org/wiki/Scriptio_continua) скриптами.

`charset_table` является основным элементом процесса токенизации Manticore, который извлекает ключевые слова из текста документа или текста запроса. Он контролирует, какие символы считаются допустимыми и как они должны преобразовываться (например, нужно ли убирать регистр).

По умолчанию каждый символ отображается в 0, что означает, что он не считается допустимым ключевым словом и рассматривается как разделитель. Как только символ упоминается в таблице, он отображается в другой символ (чаще всего либо в себя, либо в строчную букву) и считается допустимой частью ключевого слова.

charset_table использует список отображений, разделённых запятыми, чтобы объявить символы допустимыми или отобразить их в другие символы. Доступны сокращения для отображения диапазонов символов сразу:

* Отображение одного символа: `A->a`. Объявляет исходный символ 'A' допустимым в ключевых словах и отображает его в символ 'a' (но не объявляет 'a' допустимым).
* Отображение диапазона: `A..Z->a..z`. Объявляет все символы в исходном диапазоне допустимыми и отображает их в символы в целевом диапазоне. Не объявляет целевой диапазон допустимым. Проверяет длины обоих диапазонов.
* Отображение одиночного символа: `a`. Объявляет символ допустимым и отображает его в себя. Эквивалентно отображению одного символа `a->a`.
* Отображение одиночного диапазона: `a..z`. Объявляет все символы в диапазоне допустимыми и отображает их в себя. Эквивалентно отображению диапазона `a..z->a..z`.
* Отображение с шагом 2: `A..Z/2`. Отображает каждую пару символов во второй символ. Например, `A..Z/2` эквивалентно `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`. Это сокращение полезно для блоков Unicode, где заглавные и строчные буквы идут в перемешку.

Для символов с кодами от 0 до 32, а также для символов в диапазоне от 127 до 8-битных ASCII и Unicode, Manticore всегда рассматривает их как разделители. Чтобы избежать проблем с кодировкой в конфигурационных файлах, 8-битные ASCII символы и Unicode символы должны указываться в форме `U+XXX`, где `XXX` — это шестнадцатеричный номер кода символа. Минимально допустимый код символа Unicode — `U+0021`.

Если стандартных отображений недостаточно, вы можете переопределить отображения символов, указав их заново с другим отображением. Например, если встроенный массив `non_cont` включает символы `Ä` и `ä` и отображает их обоих в ASCII символ `a`, вы можете переопределить эти символы, добавив их коды Unicode, например так:

```
charset_table = non_cont,U+00E4,U+00C4
```

для поиска с учётом регистра или

```
charset_table = non_cont,U+00E4,U+00C4->U+00E4
```

для поиска без учёта регистра.

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
Помимо определения символов и отображений, доступны несколько встроенных псевдонимов, которые можно использовать. Текущие псевдонимы:
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

Если вы хотите поддерживать разные языки в поиске, определение наборов допустимых символов и правил свёртки для всех из них может быть трудоёмкой задачей. Мы упростили это для вас, предоставив таблицы символов по умолчанию — `non_cont` и `cont`, которые охватывают языки с прерывистыми и непрерывными (китайский, японский, корейский, тайский) письменностями соответственно. В большинстве случаев этих наборов символов будет достаточно для ваших нужд.

Обратите внимание, что следующие языки в настоящее время **не** поддерживаются:
* Assamese
* Bishnupriya
* Buhid
* Garo
* Hmong
* Ho
* Komi
* Large Flowery Miao
* Maba
* Maithili
* Marathi
* Mende
* Mru
* Myene
* Ngambay
* Odia
* Santali
* Sindhi
* Sylheti

Все остальные языки, перечисленные в [списке языков Unicode](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/), поддерживаются по умолчанию.
Чтобы работать как с языками с непрерывной, так и с прерывистой письменностью, установите параметры в вашем конфигурационном файле, как показано ниже (с [исключением](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md) для китайского):

<!-- example charset_table 3 -->
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'

<!-- request SQL -->

```sql
POST /cli -d "
```

<!-- request JSON -->

```JSON
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'"
$index = new \Manticoresearch\Index($client);
```

<!-- request PHP -->

```php
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'charset_table' => 'non_cont',
             'ngram_len' => '1',
             'ngram_chars' => 'cont'
        ]);
##### Python:
```
<!-- intro -->
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')

<!-- request Python -->

```python
##### Python-asyncio:
```

<!-- intro -->
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')

<!-- request Python-asyncio -->

```python
##### Javascript:
```

<!-- intro -->
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'');

<!-- request javascript -->

```javascript
##### java:
```

<!-- intro -->
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);

<!-- request Java -->

```java
##### C#:
```

<!-- intro -->
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);

<!-- request C# -->

```clike
##### Rust:
```

<!-- intro -->
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", Some(true)).await;

<!-- request Rust -->

```rust
table products {
```

<!-- request CONFIG -->

```ini
  charset_table       = non_cont
  ngram_len           = 1
  ngram_chars         = cont
  type = rt

  path = tbl
  rt_field = title
  rt_attr_uint = price
}
Если вам не нужна поддержка языков с непрерывной письменностью, вы можете просто исключить параметры [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) и [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars). Для получения дополнительной информации об этих параметрах обратитесь к соответствующим разделам документации.
```
<!-- end -->

Для отображения одного символа в несколько символов или наоборот может быть полезен [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter).
### blend_chars

blend_chars = +, &, U+23

blend_chars = +, &->+

```ini
Список смешанных символов. Необязательно, по умолчанию пусто.
Смешанные символы индексируются как разделители и как допустимые символы. Например, если `&` определён как смешанный символ и в индексируемом документе встречается `AT&T`, будут проиндексированы три разных ключевых слова: `at&t`, `at` и `t`.
```

<!-- example blend_chars -->
Кроме того, смешанные символы могут влиять на индексацию так, что ключевые слова индексируются так, как будто смешанные символы вообще не были введены. Такое поведение особенно заметно при указании `blend_mode = trim_all`. Например, фраза `some_thing` будет индексироваться как `some`, `something` и `thing` при `blend_mode = trim_all`.

При использовании смешанных символов следует быть осторожным, так как определение символа как смешанного означает, что он больше не является разделителем.

* Поэтому, если вы добавите запятую в `blend_chars` и выполните поиск по `dog,cat`, это будет рассматриваться как один токен `dog,cat`. Если `dog,cat` **не** был проиндексирован как `dog,cat`, а остался как `dog cat`, совпадения не будет.

* Следовательно, это поведение следует контролировать с помощью настройки [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode).
Позиции для токенов, полученных заменой смешанных символов на пробелы, назначаются как обычно, и обычные ключевые слова индексируются так, как если бы `blend_chars` вообще не было. Дополнительный токен, смешивающий смешанные и несмешанные символы, будет помещён на начальную позицию. Например, если в самом начале текстового поля встречается `AT&T company`, `at` получит позицию 1, `t` — позицию 2, `company` — позицию 3, а `AT&T` также получит позицию 1, смешиваясь с открывающим обычным ключевым словом. В результате запросы `AT&T` или просто `AT` найдут этот документ. Фразовый запрос `"AT T"` также совпадёт, как и фразовый запрос `"AT&T company"`.
Смешанные символы могут пересекаться со специальными символами, используемыми в синтаксисе запросов, такими как `T-Mobile` или `@twitter`. По возможности парсер запросов будет обрабатывать смешанный символ как смешанный. Например, если `hello @twitter` находится в кавычках (оператор фразы), парсер запросов обработает символ `@` как смешанный. Однако если символ `@` не в кавычках, он будет обработан как оператор. Поэтому рекомендуется экранировать ключевые слова.

Смешанные символы могут быть переназначены так, чтобы несколько разных смешанных символов нормализовались в одну базовую форму. Это полезно при индексации нескольких альтернативных кодовых точек Unicode с эквивалентными глифами.

CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_chars' => '+, &, U+23, @->_'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  blend_chars = +, &, U+23, @->_
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### blend_mode
blend_mode = option [, option [, ...]]
```
<!-- end -->

option = trim_none | trim_head | trim_tail | trim_both | trim_all | skip_pure

```ini
Режим индексации смешанных токенов включается директивой blend_mode.
По умолчанию токены, смешивающие смешанные и несмешанные символы, индексируются полностью. Например, если в `blend_chars` включены символы @ и !, строка `@dude!` будет индексироваться как два токена: `@dude!` (со всеми смешанными символами) и `dude` (без них). В результате запрос `@dude` **не** найдёт совпадений.
```

<!-- example blend_mode -->
`blend_mode` добавляет гибкости этому поведению индексации. Он принимает список опций, разделённых запятыми, каждая из которых задаёт вариант индексации токенов.

Если указано несколько опций, будет проиндексировано несколько вариантов одного и того же токена. Обычные ключевые слова (полученные из этого токена путем замены смешанных символов на разделитель) всегда индексируются.

Опции:

* `trim_none` - Индексировать весь токен

* `trim_head` - Обрезать начальные смешанные символы и индексировать полученный токен

* `trim_tail` - Обрезать конечные смешанные символы и индексировать полученный токен
* `trim_both` - Обрезать как начальные, так и конечные смешанные символы и индексировать полученный токен
* `trim_all` - Обрезать начальные, конечные и средние смешанные символы и индексировать полученный токен
* `skip_pure` - Не индексировать токен, если он состоит только из смешанных символов
Используя `blend_mode` с примером строки `@dude!` выше, настройка `blend_mode = trim_head, trim_tail` приведет к индексации двух токенов: `@dude` и `dude!`. Использование `trim_both` не даст эффекта, так как обрезка обоих смешанных символов приведет к `dude`, который уже индексируется как обычное ключевое слово. Индексация `@U.S.A.` с `trim_both` (при условии, что точка считается смешанным символом) приведет к индексации `U.S.A`. Наконец, `skip_pure` позволяет игнорировать последовательности, состоящие только из смешанных символов. Например, `one @@@ two` будет индексироваться как `one two` и будет соответствовать этой фразе. По умолчанию это не так, потому что полностью смешанный токен индексируется и смещает позицию второго ключевого слова.
Поведение по умолчанию — индексировать весь токен, что эквивалентно `blend_mode = trim_none`.

Учтите, что использование режимов смешивания ограничивает поиск, даже при режиме по умолчанию `trim_none`, если считать `.` смешанным символом:

* `.dog.` будет индексироваться как `.dog. dog`

* и вы не сможете найти его по запросу `dog.`.
Использование большего количества режимов увеличивает вероятность совпадения вашего ключевого слова.
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_mode' => 'trim_tail, skip_pure',
            'blend_chars' => '+, &'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  blend_mode = trim_tail, skip_pure
```

<!-- request CONFIG -->

```ini
  blend_chars = +, &
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### min_word_len
min_word_len = length
```
<!-- end -->

min_word_len — это необязательная опция конфигурации индекса в Manticore, которая задает минимальную длину индексируемого слова. Значение по умолчанию — 1, что означает, что индексируются все слова.

```ini
Индексируются только те слова, длина которых не меньше этого минимума. Например, если min_word_len равен 4, то слово 'the' индексироваться не будет, а 'they' — будет.
```

<!-- example min_word_len -->

CREATE TABLE products(title text, price float) min_word_len = '4'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_word_len = '4'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'min_word_len' => '4'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) min_word_len = '4'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  min_word_len = 4
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### ngram_len
ngram_len = 1
```
<!-- end -->

Длины N-грамм для индексирования N-граммами. Необязательно, значение по умолчанию 0 (отключить индексирование N-граммами). Известные значения — 0 и 1.

```ini
N-граммы обеспечивают базовую поддержку языков с непрерывным письмом в неразмеченных текстах. Проблема поиска в языках с непрерывным письмом заключается в отсутствии четких разделителей между словами. В некоторых случаях вы можете не захотеть использовать сегментацию на основе словаря, например, [как для китайского языка](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md). В таких случаях сегментация N-граммами может работать хорошо.
```

<!-- example ngram_len -->
Когда эта функция включена, потоки таких языков (или любых других символов, определенных в [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)) индексируются как N-граммы. Например, если входящий текст — "ABCDEF" (где A–F — символы какого-то языка), и ngram_len равен 1, он будет индексироваться как "A B C D E F". В настоящее время поддерживается только ngram_len=1. Только символы, перечисленные в таблице [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars), будут разбиваться таким образом; остальные не будут затронуты.

Обратите внимание, что если поисковый запрос сегментирован, то есть между отдельными словами есть разделители, то оборачивание слов в кавычки и использование расширенного режима приведет к правильному поиску совпадений, даже если текст **не** был сегментирован. Например, предположим, что исходный запрос — `BC DEF`. После оборачивания в кавычки на стороне приложения он должен выглядеть как `"BC" "DEF"` (*с* кавычками). Этот запрос будет передан в Manticore и внутренне также разбит на 1-граммы, что даст запрос `"B C" "D E F"`, все еще с кавычками, которые являются оператором поиска фразы. И он найдет совпадение в тексте, даже если в тексте не было разделителей.

Даже если поисковый запрос не сегментирован, Manticore должен выдавать хорошие результаты благодаря ранжированию на основе фраз: он поднимет выше совпадения фраз (что в случае N-грамм может означать более близкие совпадения многосимвольных слов).

CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  ngram_chars = cont
```

<!-- request CONFIG -->

```ini
  ngram_len = 1
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### ngram_chars
ngram_chars = cont
```
<!-- end -->

ngram_chars = cont, U+3000..U+2FA1F

```ini
Список символов для N-грамм. Необязательно, по умолчанию пустой.

Используется вместе с параметром [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len), этот список определяет символы, последовательности которых подлежат извлечению N-грамм. Слова, состоящие из других символов, не будут затронуты функцией индексации N-грамм. Формат значения идентичен [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Символы N-грамм не могут присутствовать в [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table).
```

<!-- example ngram_chars -->
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'U+3000..U+2FA1F',
             'ngram_len' => '1'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request javascript -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->
```java
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->
```clike
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  ngram_chars = U+3000..U+2FA1F
```

<!-- request CONFIG -->

```ini
  ngram_len = 1
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
Также вы можете использовать псевдоним для нашей стандартной таблицы N-грамм, как в примере. Это будет достаточно в большинстве случаев.
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'
```

<!-- end -->

<!-- example ngram_chars 2 -->
POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->
```java
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->
```clike
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  ngram_chars = cont
```

<!-- request CONFIG -->

```ini
  ngram_len = 1
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### ignore_chars
ignore_chars = U+AD
```
<!-- end -->

Список игнорируемых символов. Необязательно, по умолчанию пустой.

```ini
Полезно в случаях, когда некоторые символы, такие как мягкий перенос (U+00AD), должны не просто рассматриваться как разделители, а полностью игнорироваться. Например, если '-' просто отсутствует в charset_table, текст "abc-def" будет индексироваться как ключевые слова "abc" и "def". Напротив, если '-' добавлен в список ignore_chars, тот же текст будет индексироваться как одно ключевое слово "abcdef".
```

<!-- example ignore_chars -->
Синтаксис такой же, как для [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), но разрешено объявлять только символы, без их отображения. Также игнорируемые символы не должны присутствовать в charset_table.

CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'ignore_chars' => 'U+AD'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  ignore_chars = U+AD
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### bigram_index
bigram_index = {none|all|first_freq|both_freq}
```
<!-- end -->

Режим индексации биграмм. Необязательно, по умолчанию отсутствует.

```ini
Индексация биграмм — это функция для ускорения поиска фраз. При индексации в индекс сохраняется список документов для всех или некоторых пар соседних слов. Этот список затем может использоваться во время поиска для значительного ускорения сопоставления фраз или подфраз.
```

<!-- example bigram_index -->
`bigram_index` управляет выбором конкретных пар слов. Известные режимы:

* `all` — индексировать каждую пару слов

* `first_freq` — индексировать только пары слов, где *первое* слово находится в списке частотных слов (см. [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)). Например, при `bigram_freq_words = the, in, i, a` индексация текста "alone in the dark" сохранит пары "in the" и "the dark" как биграммы, потому что они начинаются с частого слова ("in" или "the"), но "alone in" не будет индексироваться, так как "in" — второе слово в паре.

* `both_freq` — индексировать только пары слов, где оба слова частые. Продолжая пример, в этом режиме при индексации "alone in the dark" будет сохранена только пара "in the" (самая плохая с точки зрения поиска), остальные пары не будут индексироваться.
Для большинства случаев `both_freq` будет лучшим режимом, но результаты могут варьироваться.
Важно отметить, что `bigram_index` работает только на уровне токенизации и не учитывает преобразования, такие как `morphology`, `wordforms` или `stopwords`. Это означает, что создаваемые токены очень просты, что делает поиск фраз более точным и строгим. Хотя это может улучшить точность сопоставления фраз, система становится менее способной распознавать разные формы слов или вариации их написания.

CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'both_freq'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  bigram_index = both_freq
```

<!-- request CONFIG -->

```ini
  bigram_freq_words = the, a, you, i
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### bigram_freq_words
bigram_freq_words = the, a, you, i
```
<!-- end -->

Список ключевых слов, считающихся "частыми" при индексации биграмм. Необязательно, по умолчанию пустой.

```ini
Некоторые режимы индексации биграмм (см. [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)) требуют определения списка часто встречающихся ключевых слов. Их **не следует** путать со стоп-словами. Стоп-слова полностью исключаются как при индексации, так и при поиске. Часто встречающиеся ключевые слова используются биграммами только для определения, индексировать ли текущую пару слов или нет.
```

<!-- example bigram_freq_words -->
`bigram_freq_words` позволяет определить такой список ключевых слов.

CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'first_freq'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  bigram_freq_words = the, a, you, i
```

<!-- request CONFIG -->

```ini
  bigram_index = first_freq
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### dict
dict = {keywords|crc}
```
<!-- end -->

Тип словаря ключевых слов определяется одним из двух известных значений: 'crc' или 'keywords'. Это необязательно, по умолчанию используется 'keywords'.

```ini
Использование режима словаря ключевых слов (dict=keywords) может значительно снизить нагрузку на индексацию и обеспечить поиск подстрок в больших коллекциях. Этот режим можно использовать как для обычных, так и для RT таблиц.
```

<!-- example dict -->
CRC-словари не хранят исходный текст ключевого слова в индексе. Вместо этого они заменяют ключевые слова контрольной суммой (вычисляемой с помощью FNV64) как при поиске, так и при индексации. Это значение используется внутри индекса. Такой подход имеет два недостатка:

* Во-первых, существует риск коллизий контрольных сумм между разными парами ключевых слов. Этот риск растет пропорционально количеству уникальных ключевых слов в индексе. Тем не менее, это незначительная проблема, так как вероятность одной коллизии FNV64 в словаре из 1 миллиарда записей примерно 1 к 16, или 6,25 процента. Большинство словарей будут содержать гораздо меньше миллиарда ключевых слов, учитывая, что типичный разговорный язык содержит от 1 до 10 миллионов словоформ.

* Во-вторых, и что более важно, с контрольными суммами сложно выполнять поиск подстрок. Manticore решил эту проблему путем предварительной индексации всех возможных подстрок как отдельных ключевых слов (см. директивы [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len), [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)). Этот метод даже имеет дополнительное преимущество — максимально быстрый поиск подстрок. Однако предварительная индексация всех подстрок значительно увеличивает размер индекса (часто в 3-10 раз и более) и, соответственно, время индексации, что делает поиск подстрок в больших индексах довольно непрактичным.
Словарь ключевых слов решает обе эти проблемы. Он хранит ключевые слова в индексе и выполняет расширение подстановочных знаков во время поиска. Например, поиск по префиксу `test*` может внутренне расширяться в запрос 'test|tests|testing' на основе содержимого словаря. Этот процесс расширения полностью прозрачен для приложения, за исключением того, что теперь также предоставляется отдельная статистика по каждому совпавшему ключевому слову.
Для поиска подстрок (инфиксного поиска) можно использовать расширенные подстановочные знаки. Специальные символы, такие как `?` и `%`, совместимы с поиском подстрок (например, `t?st*`, `run%`, `*abc*`). Обратите внимание, что [операторы подстановочных знаков](../../Searching/Full_text_matching/Operators.md#Wildcard-operators) и [оператор REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) работают только с `dict=keywords`.

Индексация с использованием словаря ключевых слов примерно в 1.1-1.3 раза медленнее, чем обычная индексация без подстрок, но значительно быстрее, чем индексация подстрок (префиксная или инфиксная). Размер индекса должен быть лишь немного больше, чем у стандартной таблицы без подстрок, с общей разницей в 1..10%. Время обычного поиска по ключевым словам должно быть почти одинаковым или идентичным для всех трех типов индексов (CRC без подстрок, CRC с подстроками, keywords). Время поиска подстрок может значительно варьироваться в зависимости от того, сколько ключевых слов соответствует заданной подстроке (то есть во сколько ключевых слов расширяется поисковый термин). Максимальное количество совпавших ключевых слов ограничено директивой [expansion_limit](../../Server_settings/Searchd.md#expansion_limit).

В итоге, словари keywords и CRC предлагают два разных компромисса для поиска подстрок. Вы можете либо пожертвовать временем индексации и размером индекса ради максимально быстрого худшего случая поиска (CRC словарь), либо минимально повлиять на время индексации, но пожертвовать временем худшего случая поиска, когда префикс расширяется в большое количество ключевых слов (keywords словарь).

CREATE TABLE products(title text, price float) dict = 'keywords'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) dict = 'keywords'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'dict' => 'keywords'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) dict = 'keywords'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  dict = keywords
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### embedded_limit
embedded_limit = size
```
<!-- end -->

Лимит размера встроенных исключений, словоформ или стоп-слов. Необязательно, по умолчанию 16K.

```ini
При создании таблицы вышеуказанные файлы могут быть либо сохранены внешне вместе с таблицей, либо встроены непосредственно в таблицу. Файлы размером меньше `embedded_limit` сохраняются в таблице. Для больших файлов сохраняются только имена файлов. Это также упрощает перенос файлов таблицы на другой компьютер; зачастую достаточно просто скопировать один файл.
```

<!-- example embedded_limit -->
С меньшими файлами такое встраивание уменьшает количество внешних файлов, от которых зависит таблица, и облегчает обслуживание. Но в то же время нет смысла встраивать словарь wordforms размером 100 МБ в крошечную дельта-таблицу. Поэтому необходим порог по размеру, и `embedded_limit` — это тот самый порог.

table products {

  embedded_limit = 32K

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### global_idf
global_idf = /path/to/global.idf
```
<!-- end -->

Путь к файлу с глобальными (по всему кластеру) IDF ключевых слов. Необязательно, по умолчанию пусто (использовать локальные IDF).

```ini
В многотабличном кластере частоты ключевых слов, скорее всего, будут отличаться в разных таблицах. Это означает, что когда функция ранжирования использует значения на основе TF-IDF, такие как факторы семейства BM25, результаты могут ранжироваться немного по-разному в зависимости от того, на каком узле кластера они находятся.
```

<!-- example global_idf -->
Самый простой способ исправить эту проблему — создать и использовать глобальный словарь частот, или сокращённо глобальный IDF-файл. Эта директива позволяет указать расположение этого файла. Рекомендуется (но не обязательно) использовать расширение .idf. Когда для данной таблицы указан IDF-файл *и* [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) установлен в 1, движок будет использовать частоты ключевых слов и количество документов коллекции из файла global_idf, а не только из локальной таблицы. Таким образом, IDF и значения, зависящие от них, будут оставаться согласованными по всему кластеру.

IDF-файлы могут использоваться несколькими таблицами. Только одна копия IDF-файла будет загружена `searchd`, даже если многие таблицы ссылаются на этот файл. Если содержимое IDF-файла изменится, новые данные можно загрузить с помощью SIGHUP.

Вы можете создать .idf файл с помощью утилиты [indextool](../../Miscellaneous_tools.md#indextool), сначала выгрузив словари с помощью переключателя `--dumpdict dict.txt --stats`, затем преобразовав их в формат .idf с помощью `--buildidf`, а затем объединив все .idf файлы по всему кластеру с помощью `--mergeidf`.

CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'global_idf' => '/usr/local/manticore/var/global.idf'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  global_idf = /usr/local/manticore/var/global.idf
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### hitless_words
hitless_words = {all|path/to/file}
```
<!-- end -->

Список слов без позиций (hitless words). Необязательно, допустимые значения — 'all' или имя файла со списком.

```ini
По умолчанию полнотекстовый индекс Manticore хранит не только список документов, соответствующих каждому ключевому слову, но и список его позиций в документе (известный как hitlist). Hitlist позволяет выполнять поиск по фразам, близости, строгому порядку и другие продвинутые типы поиска, а также ранжирование по близости фраз. Однако hitlist для некоторых часто встречающихся ключевых слов (которые по каким-то причинам нельзя исключить, несмотря на их частоту) может стать очень большим и, следовательно, замедлять обработку запросов. Кроме того, в некоторых случаях нам может быть важен только булевый поиск по ключевым словам, и позиционные операторы поиска (например, поиск по фразам) и ранжирование по фразам не нужны.
```

<!-- example hitless_words -->
`hitless_words` позволяет создавать индексы, которые либо вообще не содержат позиционной информации (hitlist), либо пропускают её для определённых ключевых слов.

Индекс без позиций обычно занимает меньше места, чем соответствующий обычный полнотекстовый индекс (ожидается примерно в 1.5 раза меньше). Индексация и поиск должны быть быстрее, но при этом отсутствует поддержка позиционных запросов и ранжирования.

Если такие слова используются в позиционных запросах (например, в запросах по фразам), то слова без позиций исключаются из них и используются как операнды без позиции. Например, если "hello" и "world" — слова без позиций, а "simon" и "says" — с позициями, то фразовый запрос `"simon says hello world"` будет преобразован в `("simon says" & hello & world)`, что означает поиск "hello" и "world" в любом месте документа и точной фразы "simon says".

Позиционный запрос, содержащий только слова без позиций, приведёт к пустому узлу фразы, поэтому весь запрос вернёт пустой результат и предупреждение. Если весь словарь без позиций (используется `all`), то на соответствующем индексе можно использовать только булевый поиск.

CREATE TABLE products(title text, price float) hitless_words = 'all'

POST /cli -d "



<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) hitless_words = 'all'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'hitless_words' => 'all'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  hitless_words = all
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### index_field_lengths
index_field_lengths = {0|1}
```
<!-- end -->

Включает вычисление и сохранение длин полей (как для каждого документа, так и средних значений по индексу) в полнотекстовом индексе. Необязательно, по умолчанию 0 (не вычислять и не сохранять).

```ini
Когда `index_field_lengths` установлен в 1, Manticore будет:
```

<!-- example index_field_lengths -->
* создавать соответствующий атрибут длины для каждого полнотекстового поля с тем же именем, но с суффиксом `__len`

* вычислять длину поля (в количестве ключевых слов) для каждого документа и сохранять в соответствующий атрибут
* вычислять средние значения по индексу. Атрибуты длины будут иметь специальный тип TOKENCOUNT, но их значения на самом деле являются обычными 32-битными целыми числами, и к ним обычно можно получить доступ.
[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) и [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) функции в ранжировщике выражений основаны на этих длинах и требуют включения `index_field_lengths`. Исторически Manticore использовал упрощённый, урезанный вариант BM25, который, в отличие от полной функции, **не** учитывал длину документа. Также поддерживается как полный вариант BM25, так и его расширение для нескольких полей, называемое BM25F. Они требуют длины на документ и длины на поле соответственно. Отсюда и дополнительная директива.
CREATE TABLE products(title text, price float) index_field_lengths = '1'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_field_lengths = '1'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_field_lengths' => '1'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  index_field_lengths = 1
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### index_token_filter
index_token_filter = my_lib.so:custom_blend:chars=@#&
```
<!-- end -->

Фильтр токенов во время индексации для полнотекстового индексирования. Опционально, по умолчанию пусто.

```ini
Директива index_token_filter задаёт опциональный фильтр токенов во время индексации для полнотекстового индексирования. Эта директива используется для создания пользовательского токенизатора, который формирует токены согласно пользовательским правилам. Фильтр создаётся индексатором при индексации исходных данных в обычную таблицу или RT-таблицей при обработке операторов `INSERT` или `REPLACE`. Плагины определяются в формате `имя_библиотеки:имя_плагина:опциональная_строка_настроек`. Например, `my_lib.so:custom_blend:chars=@#&`.
```

<!-- example index_token_filter -->
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_token_filter' => 'my_lib.so:custom_blend:chars=@#&'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  index_token_filter = my_lib.so:custom_blend:chars=@#&
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### overshort_step
overshort_step = {0|1}
```
<!-- end -->

Инкремент позиции для слишком коротких (меньше [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)) ключевых слов. Опционально, допустимые значения 0 и 1, по умолчанию 1.

```ini
CREATE TABLE products(title text, price float) overshort_step = '1'
```

<!-- example overshort_step -->
POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) overshort_step = '1'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'overshort_step' => '1'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) overshort_step = '1'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  overshort_step = 1
```

<!-- request CONFIG -->

```ini
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### phrase_boundary
phrase_boundary = ., ?, !, U+2026 # horizontal ellipsis
```
<!-- end -->

Список символов границ фраз. Опционально, по умолчанию пусто.

```ini
Этот список контролирует, какие символы будут рассматриваться как границы фраз, чтобы корректировать позиции слов и включить эмуляцию поиска на уровне фраз через поиск по близости. Синтаксис похож на [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), но отображения не разрешены, и символы границ не должны пересекаться с чем-либо ещё.
```

<!-- example phrase_boundary -->
На границе фразы к текущей позиции слова будет добавлен дополнительный инкремент позиции слова (указанный в [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)). Это позволяет выполнять поиск на уровне фраз через запросы по близости: слова из разных фраз гарантированно будут находиться на расстоянии больше phrase_boundary_step друг от друга; таким образом, поиск по близости в пределах этого расстояния будет эквивалентен поиску на уровне фраз.

Условие границы фразы будет срабатывать только если за таким символом следует разделитель; это сделано, чтобы избежать обработки сокращений типа S.T.A.L.K.E.R или URL как нескольких фраз.

CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary' => '., ?, !, U+2026',
             'phrase_boundary_step' => '10'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')

```
<!-- intro -->
##### Pytho-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  phrase_boundary = ., ?, !, U+2026
```

<!-- request CONFIG -->

```ini
  phrase_boundary_step = 10
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### phrase_boundary_step
phrase_boundary_step = 100
```
<!-- end -->

Инкремент позиции слова на границе фразы. Опционально, по умолчанию 0.

```ini
На границе фразы текущая позиция слова будет дополнительно увеличена на это число.
```

<!-- example phrase_boundary_step -->
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary_step' => '100',
             'phrase_boundary' => '., ?, !, U+2026'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')

```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```
<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  phrase_boundary_step = 100
```

<!-- request CONFIG -->

```ini
  phrase_boundary = ., ?, !, U+2026
  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
### regexp_filter
# index '13"' as '13inch'
```
<!-- end -->

regexp_filter = \b(\d+)\" => \1inch

```ini
# index 'blue' or 'red' as 'color'
regexp_filter = (blue|red) => color

Регулярные выражения (regexps), используемые для фильтрации полей и запросов. Эта директива является необязательной, может принимать несколько значений, и по умолчанию представляет собой пустой список регулярных выражений. Движок регулярных выражений, используемый в Manticore Search, — это RE2 от Google, известный своей скоростью и безопасностью. Для подробной информации о синтаксисе, поддерживаемом RE2, вы можете посетить [руководство по синтаксису RE2](https://github.com/google/re2/wiki/Syntax).
В некоторых приложениях, таких как поиск товаров, может быть много способов обозначить продукт, модель или свойство. Например, `iPhone 3gs` и `iPhone 3 gs` (или даже `iPhone3 gs`) с большой вероятностью относятся к одному и тому же продукту. Другой пример — различные способы указания размера экрана ноутбука, такие как `13-inch`, `13 inch`, `13"`, или `13in`.
```

<!-- example regexp_filter -->
Регулярные выражения предоставляют механизм для задания правил, адаптированных для обработки таких случаев. В первом примере вы могли бы использовать файл wordforms для обработки нескольких моделей iPhone, но во втором примере лучше задать правила, которые нормализуют "13-inch" и "13in" к одному и тому же виду.

Регулярные выражения, перечисленные в `regexp_filter`, применяются в том порядке, в котором они указаны, на самом раннем этапе, до любой другой обработки (включая [исключения](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)), даже до токенизации. То есть, регулярные выражения применяются к исходным полям при индексации и к исходному тексту поискового запроса при поиске.

CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'

POST /cli -d "

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'"
```

<!-- request JSON -->

```JSON
$index = new \Manticoresearch\Index($client);
$index->setName('products');
```

<!-- request PHP -->

```php
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'regexp_filter' => '(blue|red) => color'
        ]);
##### Python:
utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')

```
<!-- intro -->
##### Python-asyncio:

<!-- request Python -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')
```

<!-- intro -->
##### Javascript:

<!-- request Python-asyncio -->

```python
res = await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'');
```

<!-- intro -->
##### java:

<!-- request javascript -->

```javascript
utilsApi.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### C#:

<!-- request Java -->

```java
utilsApi.Sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### Rust:

<!-- request C# -->

```clike
utils_api.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", Some(true)).await;
```

<!-- intro -->
table products {

<!-- request Rust -->

```rust
  # index '13"' as '13inch'
```

<!-- request CONFIG -->

```ini
  regexp_filter = \b(\d+)\" => \1inch
  # index 'blue' or 'red' as 'color'
  regexp_filter = (blue|red) => color

  type = rt
  path = tbl

  rt_field = title
  rt_attr_uint = price
}
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->

