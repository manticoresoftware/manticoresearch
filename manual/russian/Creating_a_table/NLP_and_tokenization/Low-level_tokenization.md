# Низкоуровневая токенизация

Когда текст индексируется в Manticore, он разбивается на слова, и выполняется сведение регистра так, чтобы слова, такие как "Abc", "ABC" и "abc", рассматривались как одно и то же слово.

Для правильного выполнения этих операций Manticore должен знать:
* кодировку исходного текста (которая всегда должна быть UTF-8)
* какие символы считаются буквами, а какие — нет
* какие буквы должны сводиться к другим буквам

Вы можете настроить эти параметры для каждой таблицы с помощью опции [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). charset_table задаёт массив, который сопоставляет буквенные символы с их версиями в приведённом к одному регистру виде (или с любыми другими предпочитаемыми символами). Символы, отсутствующие в массиве, считаются не буквами и будут обрабатываться как разделители слов при индексации или поиске в этой таблице.

Набор символов по умолчанию — `non_cont`, который включает [большинство языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md).

Вы также можете определить правила замены текстовых шаблонов. Например, с такими правилами:

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

Текст `RED TUBE 5" LONG` будет индексироваться как `COLOR TUBE 5 INCH LONG`, а `PLANK 2" x 4"` будет индексироваться как `PLANK 2 INCH x 4 INCH`. Эти правила применяются в указанном порядке. Правила также применяются к запросам, поэтому поиск по `BLUE TUBE` фактически будет искать `COLOR TUBE`.

Более подробно о [regexp_filter можно узнать здесь](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter).

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
`charset_table` задаёт массив, который сопоставляет буквенные символы с их версиями в приведённом к одному регистру виде (или с любыми другими символами, если вы предпочитаете). Набор символов по умолчанию — `non_cont`, который включает большинство языков с [неконтинуальными](https://en.wikipedia.org/wiki/Scriptio_continua) сценариями письма.

`charset_table` является ключевым элементом процесса токенизации в Manticore, который извлекает ключевые слова из текста документа или текста запроса. Он контролирует, какие символы считаются допустимыми и как они должны преобразовываться (например, следует ли удалять регистр или нет).

По умолчанию каждый символ отображается в 0, что означает, что он не считается допустимым ключевым словом и обрабатывается как разделитель. Как только символ упоминается в таблице, он отображается в другой символ (чаще всего в себя самого или в строчную букву) и считается допустимой частью ключевого слова.

charset_table использует список отображений, разделённых запятыми, чтобы объявить символы допустимыми или сопоставить их с другими символами. Существуют сокращённые синтаксисы для сопоставления диапазонов символов сразу:

* Отображение одного символа: `A->a`. Объявляет исходный символ 'A' допустимым в ключевых словах и сопоставляет его с символом назначения 'a' (но не объявляет 'a' как допустимый).
* Отображение диапазона: `A..Z->a..z`. Объявляет все символы из исходного диапазона допустимыми и сопоставляет их с соответствующими символами из целевого диапазона. Не объявляет целевой диапазон как допустимый. Проверяется равенство длины диапазонов.
* Отображение одиночного символа: `a`. Объявляет символ допустимым и сопоставляет его с самим собой. Эквивалентно отображению одного символа `a->a`.
* Отображение одиночного диапазона: `a..z`. Объявляет все символы в диапазоне допустимыми и сопоставляет их сами с собой. Эквивалентно отображению диапазона `a..z->a..z`.
* Отображение с шахматной доской: `A..Z/2`. Сопоставляет каждую пару символов со вторым символом в паре. Например, `A..Z/2` эквивалентно `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`. Этот сокращённый способ удобен для Unicode-блоков, где заглавные и прописные буквы чередуются.

Для символов с кодами от 0 до 32 и символов в диапазоне от 127 до 8-битного ASCII и Unicode символов, Manticore всегда рассматривает их как разделители. Чтобы избежать проблем с кодировкой файла конфигурации, 8-битные ASCII и Unicode символы должны указываться в форме `U+XXX`, где `XXX` — шестнадцатеричный номер кода символа. Минимальный допустимый код Unicode — `U+0021`.

Если стандартных отображений недостаточно, вы можете переопределить отображения символов, указав их заново с другим отображением. Например, если встроенный массив `non_cont` включает символы `Ä` и `ä` и сопоставляет их обоих с ASCII символом `a`, вы можете переопределить эти символы, добавив их Unicode-коды, так:

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
Помимо определений символов и отображений, имеются несколько встроенных псевдонимов, которые можно использовать. Текущие псевдонимы:
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

Если вы хотите поддерживать различные языки в вашем поиске, определение наборов допустимых символов и правил свертки для всех языков может оказаться трудоемкой задачей. Мы упростили эту задачу для вас, предоставив стандартные таблицы наборов символов, `non_cont` и `cont`, которые охватывают языки с неконтинуальными и континуальными (китайский, японский, корейский, тайский) системами письма соответственно. В большинстве случаев этих наборов должно быть достаточно для ваших нужд.

Обратите внимание, что следующие языки в настоящее время **не** поддерживаются:
* Ассамский
* Бишнуприя
* Бухид
* Гаро
* Хмонг
* Хо
* Коми
* Большой Цветочный Мяо
* Маба
* Майтхили
* Марати
* Менде
* Мру
* Мьене
* Нгамбай
* Одиа
* Сантали
* Синдхи
* Силхети

Все остальные языки, перечисленные в [списке языков Unicode
list](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/), поддерживаются по умолчанию.

<!-- example charset_table 3 -->
Чтобы работать с языками как с континуальным, так и с неконтинуальным письмом, установите опции в вашем конфигурационном файле, как показано ниже (с [исключением](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md) для китайского):

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

Если вам не требуется поддержка языков с континуальным письмом, можно просто не включать параметры [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) и [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars).
Для получения дополнительной информации об этих параметрах обратитесь к соответствующим разделам документации.

Чтобы сопоставить один символ с несколькими или наоборот, можно использовать опцию [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter).

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
Список смешанных символов. Необязательно, по умолчанию пусто.

Смешанные символы индексируются и как разделители, и как допустимые символы. Например, если `&` определён как смешанный символ и в индексируемом документе встречается `AT&T`, то будут проиндексированы три разных ключевых слова: `at&t`, `at` и `t`.

Кроме того, смешанные символы могут влиять на индексацию так, что ключевые слова индексируются как если бы смешанных символов не было вовсе. Это особенно заметно, если задано `blend_mode = trim_all`. Например, фраза `some_thing` будет индексироваться как `some`, `something` и `thing` с `blend_mode = trim_all`.

Следует соблюдать осторожность при использовании смешанных символов, так как определение символа как смешанного означает, что он больше не считается разделителем.
* Поэтому если добавить запятую в `blend_chars` и выполнить поиск по `dog,cat`, это будет считаться одним токеном `dog,cat`. Если `dog,cat` **не** был проиндексирован как `dog,cat`, а остался только как `dog cat`, то поисковый запрос не совпадёт.
* Следовательно, это поведение следует регулировать с помощью параметра [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode).

Позиции для токенов, полученных путем замены смешанных символов пробелами, назначаются как обычно, и обычные ключевые слова будут индексироваться так, как если бы не было определено ни одного `blend_chars`. Дополнительно, токен, сочетающий смешанные и обычные символы, будет помещен в начальную позицию. Например, если `AT&T company` встречается в самом начале текстового поля, `at` получит позицию 1, `t` — позицию 2, `company` — позицию 3, а `AT&T` также будет иметь позицию 1, совпадая с обычным открывающим ключевым словом. В результате запросы по `AT&T` или просто `AT` совпадут с этим документом. Запрос-фраза `"AT T"` также даст совпадение, как и запрос-фраза `"AT&T company"`.

Смешанные символы могут совпадать со специальными символами, используемыми в синтаксисе запросов, такими как `T-Mobile` или `@twitter`. Где возможно, парсер запросов обработает смешанный символ как смешанный. Например, если `hello @twitter` заключено в кавычки (оператор фразы), парсер обработает символ `@` как смешанный. Однако если символ `@` находится вне кавычек, он будет рассматриваться как оператор. Поэтому рекомендуется экранировать ключевые слова.

Смешанные символы могут быть отображены так, чтобы несколько разных смешанных символов нормализовались до одной базовой формы. Это полезно при индексации различных альтернативных Unicode-кодпоинтов с эквивалентными глифами.

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

По умолчанию токены, содержащие смешанные и обычные символы, индексируются полностью. Например, если и собачка, и восклицательный знак указаны в `blend_chars`, строка `@dude!` будет проиндексирована как два токена: `@dude!` (со всеми смешанными символами) и `dude` (без них). В результате запрос по `@dude` **не** совпадет.

`blend_mode` добавляет гибкости этому поведению индексации. Он принимает список опций, разделённых запятыми, каждая из которых указывает вариант индексации токенов.

Если указано несколько опций, будут проиндексированы несколько вариантов одного и того же токена. Обычные ключевые слова (получаемые из этого токена заменой смешанных символов на разделитель) всегда индексируются.

Опции:

* `trim_none` - Индексировать весь токен целиком
* `trim_head` - Обрезать смешанные символы в начале и индексировать полученный токен
* `trim_tail` - Обрезать смешанные символы в конце и индексировать полученный токен
* `trim_both`- Обрезать смешанные символы и в начале, и в конце, и индексировать полученный токен
* `trim_all` - Обрезать смешанные символы в начале, конце и середине, и индексировать полученный токен
* `skip_pure` - Не индексировать токен, если он состоит только из смешанных символов

Использование `blend_mode` на примере строки `@dude!` выше, с настройкой `blend_mode = trim_head, trim_tail` приведет к индексации двух токенов: `@dude` и `dude!`. Использование `trim_both` не даст эффекта, потому что обрезка смешанных символов в начале и конце даёт `dude`, который уже индексируется как обычное ключевое слово. Индексация `@U.S.A.` с `trim_both` (с учетом, что точка – смешанный символ 2-го типа) приведет к индексации `U.S.A`. Наконец, `skip_pure` позволяет игнорировать последовательности только из смешанных символов. Например, `one @@@ two` будет индексироваться как `one two` и соответствовать этой фразе. По умолчанию так не происходит, так как полностью смешанный токен индексируется и смещает позицию второго ключевого слова.

Поведение по умолчанию - индексировать весь токен, что эквивалентно `blend_mode = trim_none`.

Учтите, что использование режимов смешивания ограничивает поиск, даже при стандартном режиме `trim_none`, если считать `.` смешанным символом:
* `.dog.` при индексации превратится в `.dog. dog`
* и вы не сможете найти это через `dog.`.

Использование более чем одного режима повышает вероятность совпадения ключевого слова с чем-то.

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

min_word_len — это необязательная опция конфигурации индекса в Manticore, задающая минимальную длину индексируемого слова. Значение по умолчанию — 1, что означает, что индексируются все слова.

Индексации подлежат только те слова, длина которых не короче этого минимального значения. Например, если min_word_len равен 4, слово 'the' индексироваться не будет, а слово 'they' — будет.

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
Длины N-грамм для индексации N-граммами. Необязательно, значение по умолчанию 0 (отключить индексацию N-граммами). Известны значения 0 и 1.

N-граммы обеспечивают базовую поддержку языков с непрерывной письменностью в неразмеченных текстах. Проблема поиска в языках с непрерывной письменностью — отсутствие четких разделителей между словами. В некоторых случаях может не подойти сегментация на основе словаря, например, [та, что доступна для китайского](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md). В таких случаях сегментация с помощью N-грамм может также хорошо работать.

Если эта возможность включена, потоки такого рода языков (или любые другие символы, определённые в таблице [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)) индексируются как N-граммы. Например, если текст "ABCDEF" (где A–F — символы языка) и ngram_len равен 1, это будет проиндексировано как "A B C D E F". В настоящее время поддерживается только ngram_len=1. Только символы из таблицы [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) будут разбиваться таким образом; остальные останутся без изменений.

Обратите внимание, что если поисковый запрос сегментирован, то есть между словами есть разделители, то обертывание слов в кавычки и использование расширенного режима приведёт к нахождению правильных совпадений, даже если текст **не был** сегментирован. Например, предположим исходный запрос `BC DEF`. После обёртки в кавычки на стороне приложения запрос будет выглядеть как `"BC" "DEF"` (*с* кавычками). Этот запрос передается в Manticore и внутри тоже разбивается на 1-граммы, задавая запрос `"B C" "D E F"`, при этом кавычки сохраняются как оператор фразового сопоставления. И он найдёт соответствия в тексте, даже если в тексте не было разделителей.

Даже если поисковый запрос не сегментирован, Manticore должен давать хорошие результаты благодаря ранжированию с учётом фраз: он будет выводить ближайшие фразовые совпадения (что для слов из N-грамм означает совпадения с рядом идущими символами) выше в списке.

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
Список символов для N-грамм. Необязательно, по умолчанию пусто.

Используется вместе с параметром [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len), этот список определяет символы, последовательности которых подвергаются извлечению N-грамм. Слова, состоящие из других символов, не будут затронуты функцией индексирования N-грамм. Формат значения идентичен [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Символы N-грамм не могут присутствовать в [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table).

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
Также вы можете использовать псевдоним для нашей стандартной таблицы N-грамм, как в примере. В большинстве случаев этого будет достаточно.

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
Список игнорируемых символов. Необязательно, по умолчанию пусто.

Полезно в случаях, когда некоторые символы, например, мягкий перенос (U+00AD), следует не просто рассматривать как разделители, а полностью игнорировать. Например, если '-' просто отсутствует в charset_table, текст "abc-def" будет индексироваться как ключевые слова "abc" и "def". Напротив, если '-' добавлен в список ignore_chars, тот же текст будет индексироваться как одно ключевое слово "abcdef".

Синтаксис такой же, как для [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), но разрешено объявлять только символы, а не сопоставлять их. Также игнорируемые символы не должны присутствовать в charset_table.

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

Индексация биграмм — это функция для ускорения поиска фраз. При индексации она сохраняет список документов для всех или некоторых пар соседних слов в индексе. Такой список затем может использоваться во время поиска для значительного ускорения поиска фраз или их частей.

`bigram_index` управляет выбором конкретных пар слов. Известны следующие режимы:

* `all` — индексировать каждую пару слов
* `first_freq` — индексировать только пары слов, где *первое* слово находится в списке частотных слов (см. [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)). Например, с `bigram_freq_words = the, in, i, a` при индексации текста "alone in the dark" будут сохранены пары "in the" и "the dark" как биграммы, потому что они начинаются с частого слова ("in" или "the"), а "alone in" не будет индексирована, так как "in" — второе слово в паре.
* `both_freq` — индексировать только пары слов, где оба слова частотные. Продолжая тот же пример, в этом режиме при индексации "alone in the dark" будет сохранена только пара "in the" (самая плохая для поиска), остальные пары не будут индексированы.

Для большинства случаев лучший режим — `both_freq`, но всё зависит от ваших задач.

Важно отметить, что `bigram_index` работает только на уровне токенизации и не учитывает преобразования как `morphology`, `wordforms` или `stopwords`. Это означает, что создаваемые токены очень простые, что делает поиск фраз более точным и строгим. Хотя это может повысить точность фразового совпадения, система становится менее способной распознавать различные формы слов или вариации их появления.

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
Список ключевых слов, считаемых "частотными" при индексировании биграмм. Необязательно, по умолчанию пусто.

Некоторые режимы индексирования биграмм (см. [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)) требуют определения списка частых ключевых слов. Их **не** следует путать со стоп-словами. Стоп-слова полностью исключаются как при индексировании, так и при поиске. Частые ключевые слова используются биграммами только для определения необходимости индексировать текущую пару слов или нет.

`bigram_freq_words` позволяет определить такой список ключевых слов.

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
Тип словаря ключевых слов определяется одним из двух известных значений: 'crc' или 'keywords'. Это опционально, по умолчанию используется 'keywords'.

Использование режима словаря ключевых слов (dict=keywords) может значительно снизить нагрузку индексирования и позволить выполнять поиск подстрок в крупных коллекциях. Этот режим может применяться как для обычных, так и для RT таблиц.

CRC-словари не хранят оригинальный текст ключевых слов в индексе. Вместо этого они заменяют ключевые слова на значение контрольной суммы (вычисляемой с помощью FNV64) как при поиске, так и при индексировании. Это значение используется внутри индекса. У этого подхода есть два недостатка:
* Во-первых, существует риск коллизий контрольных сумм между разными парами ключевых слов. Этот риск растет пропорционально количеству уникальных ключевых слов в индексе. Тем не менее, эта проблема незначительна, поскольку вероятность одной коллизии FNV64 в словаре из 1 миллиарда записей примерно 1 из 16, или 6,25 процента. Большинство словарей содержит гораздо меньше ключевых слов, учитывая, что типичный живой язык имеет от 1 до 10 миллионов форм слов.
* Во-вторых, и что важнее, с контрольными суммами сложно выполнять поиск подстрок. Manticore решила эту проблему предварительным индексированием всех возможных подстрок как отдельных ключевых слов (см. директивы [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len), [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)). Этот метод даже имеет дополнительное преимущество — поиск подстрок осуществляется максимально быстро. Однако предварительное индексирование всех подстрок значительно увеличивает размер индекса (часто в 3-10 раз и более) и, соответственно, время индексирования, что делает поиск подстрок в больших индексах малопрактичным.

Словарь ключевых слов решает обе указанные проблемы. Он хранит ключевые слова в индексе и выполняет расширение подстановочных символов во время поиска. Например, поиск префикса `test*` может внутренне расшириться до запроса 'test|tests|testing' на основе содержимого словаря. Этот процесс расширения полностью прозрачен для приложения, за исключением того, что теперь также отображается отдельная статистика по каждому из совпавших ключевых слов.

Для поиска подстрок (инфиксов) можно использовать расширенные шаблоны с подстановочными символами. Специальные символы, такие как `?` и `%`, совместимы с поиском подстрок (например, `t?st*`, `run%`, `*abc*`). Обратите внимание, что [операторы подстановок](../../Searching/Full_text_matching/Operators.md#Wildcard-operators) и [REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) работают только с `dict=keywords`.

Индексирование с использованием словаря ключевых слов примерно в 1.1-1.3 раза медленнее обычного индексирования без подстрок — но существенно быстрее индексирования с подстроками (как префиксными, так и инфиксными). Размер индекса должен быть лишь немного больше, чем у стандартной таблицы без подстрок, с общей разницей от 1 до 10%. Время обычного поиска по ключевым словам должно быть почти одинаковым во всех трёх рассмотренных типах индексов (CRC без подстрок, CRC с подстроками, keywords). Время поиска подстрок может значительно варьироваться в зависимости от количества ключевых слов, совпадающих с данной подстрокой (то есть насколько сильно поисковый термин расширяется). Максимальное число совпадающих ключевых слов ограничено директивой [expansion_limit](../../Server_settings/Searchd.md#expansion_limit).

В итоге, словари ключевых слов и CRC предлагают два разных варианта компромиссов для поиска подстрок. Вы можете либо пожертвовать временем и размером индексирования ради максимально быстрого в худшем случае поиска (CRC-словарь), либо минимально повлиять на время индексирования, но пожертвовать скоростью в худших случаях, когда префикс расширяется до большого числа ключевых слов (словарь ключевых слов).

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
Лимит размера файла встроенных исключений, форм слов или стоп-слов. Опционально, по умолчанию 16К.

При создании таблицы указанные выше файлы могут сохраняться либо внешне вместе с таблицей, либо непосредственно в самой таблице. Файлы размером меньше `embedded_limit` сохраняются внутри таблицы. Для больших файлов сохраняются только имена файлов. Это также упрощает перенос файлов таблицы на другой компьютер; зачастую достаточно просто скопировать один файл.

При небольших файлах такое встраивание уменьшает количество внешних файлов, от которых зависит таблица, и упрощает поддержку. Однако в то же время нет смысла встраивать словарь словоформ размером 100 МБ в крошечную дельта-таблицу. Поэтому необходим порог размера, и `embedded_limit` является этим порогом.

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
Путь к файлу с глобальными (для всего кластера) IDF ключевых слов. Необязательный параметр, по умолчанию пустой (используются локальные IDF).

В кластере с несколькими таблицами частоты ключевых слов, скорее всего, будут различаться в разных таблицах. Это означает, что когда функция ранжирования использует значения на основе TF-IDF, такие как семейство факторов BM25, результаты могут ранжироваться немного по-разному в зависимости от того, на каком узле кластера они находятся.

Самый простой способ решить эту проблему — создать и использовать глобальный словарь частот, или, сокращенно, глобальный IDF файл. Эта директива позволяет указать расположение этого файла. Рекомендуется (но не обязательно) использовать расширение .idf. Когда для заданной таблицы указан файл IDF *и* установлена опция [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) равной 1, движок будет использовать частоты ключевых слов и количество документов в коллекции из файла global_idf, а не только из локальной таблицы. Таким образом, IDF и зависящие от них значения останутся согласованными по всему кластеру.

Файлы IDF могут использоваться несколькими таблицами совместно. Только одна копия файла IDF будет загружена `searchd`, даже если на этот файл ссылаются многие таблицы. Если содержимое файла IDF изменится, новое содержимое можно загрузить с помощью SIGHUP.

Вы можете создать файл .idf с помощью утилиты [indextool](../../Miscellaneous_tools.md#indextool), сначала сдампив словари с использованием ключа `--dumpdict dict.txt --stats`, затем преобразовав их в формат .idf с помощью `--buildidf`, а затем объединив все .idf файлы по кластеру с помощью `--mergeidf`.

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
Список слов без позиций. Необязательный параметр, допустимые значения: 'all' или имя файла со списком.

По умолчанию полнотекстовый индекс Manticore хранит не только список соответствующих документов для каждого заданного ключевого слова, но и список его позиций в документе (известный как хитлист). Хитлисты позволяют выполнять фразовый, proximity, строгий порядок и другие продвинутые типы поиска, а также ранжирование по близости фраз. Однако хитлисты для определенных частых ключевых слов (которые по какой-то причине не могут быть стоп-словами, несмотря на частоту) могут стать огромными и, следовательно, медленными для обработки при запросе. Кроме того, в некоторых случаях нас может интересовать только булево соответствие ключевых слов, и никогда не потребуются операторы поиска на основе позиций (такие как фразовый поиск) или ранжирование по фразам.

`hitless_words` позволяет создавать индексы, которые либо вообще не имеют позиционной информации (хитлистов), либо пропускают её для конкретных ключевых слов.

Индекс без позиций, как правило, будет занимать меньше места, чем соответствующий обычный полнотекстовый индекс (можно ожидать примерно в 1,5 раза меньше). Индексирование и поиск должны быть быстрее, ценой потери поддержки позиционных запросов и ранжирования.

Если такие слова используются в позиционных запросах (например, фразовых запросах), они извлекаются из них и используются как операнд без позиции. Например, если "hello" и "world" являются словами без позиций, а "simon" и "says" — нет, то фразовый запрос `"simon says hello world"` будет преобразован в `("simon says" & hello & world)`, где "hello" и "world" могут находиться в любом месте документа, а "simon says" — как точная фраза.

Позиционный запрос, содержащий только слова без позиций, приведет к созданию пустого фразового узла, поэтому весь запрос вернет пустой результат и предупреждение. Если весь словарь состоит из слов без позиций (с использованием `all`), то в соответствующем индексе можно использовать только булев поиск.



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

### hitless_words_list

```ini
hitless_words_list = 'word1; word2; ...'
```

<!-- example hitless_words_list -->
Настройка `hitless_words_list` позволяет указать слова без позиций непосредственно в операторе `CREATE TABLE`. Поддерживается только в [RT режиме](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29).

Значения должны быть разделены точкой с запятой (`;`).

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'hitless_words_list' => 'hello; world'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words_list = \'hello; world\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words_list = \'hello; world\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words_list = \'hello; world\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) hitless_words_list = 'hello; world'", Some(true)).await;
```

<!-- end -->

### index_field_lengths

```ini
index_field_lengths = {0|1}
```

<!-- example index_field_lengths -->
Включает вычисление и сохранение длин полей (как для каждого документа, так и средних значений по индексу) в полнотекстовый индекс. Необязательный параметр, по умолчанию равен 0 (не вычислять и не сохранять).

Когда `index_field_lengths` установлен в 1, Manticore будет:
* создавать соответствующий атрибут длины для каждого полнотекстового поля, использующий то же имя, но с суффиксом `__len`
* вычислять длину поля (подсчитываемую в ключевых словах) для каждого документа и сохранять её в соответствующий атрибут
* вычислять средние значения по индексу. Атрибуты длин будут иметь специальный тип TOKENCOUNT, но их значения фактически являются обычными 32-битными целыми числами, и к ним можно обращаться.

Функции [BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) и [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) в ранкере выражений основаны на этих длинах и требуют включения `index_field_lengths`. Исторически Manticore использовал упрощённый, урезанный вариант BM25, который, в отличие от полной функции, **не** учитывал длину документа. Также поддерживается как полный вариант BM25, так и его расширение для нескольких полей, называемое BM25F. Они требуют, соответственно, длины каждого документа и длин каждого поля. Отсюда и дополнительная директива.

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
Токенный фильтр на этапе индексации для полнотекстового индексирования. Необязательный параметр, по умолчанию пуст.

Директива index_token_filter задаёт необязательный токенный фильтр на этапе индексации для полнотекстового индексирования. Эта директива используется для создания пользовательского токенизатора, который формирует токены в соответствии с пользовательскими правилами. Фильтр создаётся индексатором при индексировании исходных данных в обычную таблицу или RT-таблицей при обработке операторов `INSERT` или `REPLACE`. Плагины определяются с использованием формата `имя_библиотеки:имя_плагина:необязательная_строка_настроек`. Например, `my_lib.so:custom_blend:chars=@#&`.

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
Приращение позиции для слишком коротких (меньше [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)) ключевых слов. Необязательный параметр, допустимые значения 0 и 1, по умолчанию 1.

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
Список символов границ фраз. Необязательный параметр, по умолчанию пуст.

Этот список управляет тем, какие символы будут рассматриваться как границы фраз, чтобы корректировать позиции слов и включать эмуляцию поиска по фразам через поиск по близости. Синтаксис похож на [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), но сопоставления не допускаются, и символы границ не должны пересекаться ни с чем другим.

На границе фразы к текущей позиции слова будет добавлено дополнительное приращение позиции (указанное в [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)). Это позволяет выполнять поиск на уровне фраз через запросы близости: слова из разных фраз гарантированно будут находиться на расстоянии больше, чем phrase_boundary_step, друг от друга; поэтому поиск близости в пределах этого расстояния будет эквивалентен поиску по фразам.

Условие границы фразы будет срабатывать тогда и только тогда, когда за таким символом следует разделитель; это сделано для того, чтобы избежать трактовки сокращений, таких как S.T.A.L.K.E.R, или URL-адресов как нескольких фраз.

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
Приращение позиции слова на границе фразы. Необязательный параметр, по умолчанию 0.

На границе фразы текущая позиция слова будет дополнительно увеличена на это число.

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
Регулярные выражения (regexps), используемые для фильтрации полей и запросов. Эта директива необязательна, может иметь несколько значений, и по умолчанию представляет собой пустой список регулярных выражений. Движок регулярных выражений, используемый Manticore Search, — это RE2 от Google, известный своей скоростью и безопасностью. Подробную информацию о синтаксисе, поддерживаемом RE2, можно найти в [руководстве по синтаксису RE2](https://github.com/google/re2/wiki/Syntax).

В некоторых приложениях, таких как поиск товаров, может быть множество способов указать на продукт, модель или свойство. Например, `iPhone 3gs` и `iPhone 3 gs` (или даже `iPhone3 gs`) с большой вероятностью относятся к одному и тому же продукту. Другой пример — различные способы обозначения размера экрана ноутбука, такие как `13-inch`, `13 inch`, `13"` или `13in`.

Регулярные выражения предоставляют механизм для задания правил, адаптированных для обработки таких случаев. В первом примере можно было бы использовать файл wordforms для обработки нескольких моделей iPhone, но во втором примере лучше задать правила, которые приведут "13-inch" и "13in" к чему-то идентичному.

Регулярные выражения, перечисленные в `regexp_filter`, применяются в том порядке, в котором они указаны, на максимально ранней стадии, до любой другой обработки (включая [исключения](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)), даже до токенизации. То есть regexps применяются к исходным полям при индексации и к исходному тексту поискового запроса при поиске.

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

