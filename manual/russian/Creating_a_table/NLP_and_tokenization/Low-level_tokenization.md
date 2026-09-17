# Низкоуровневая токенизация

Когда текст индексируется в Manticore, он разбивается на слова, а затем выполняется приведение регистра, чтобы слова вроде "Abc", "ABC" и "abc" считались одним и тем же словом.

Чтобы корректно выполнять эти операции, Manticore должно знать:
* кодировку исходного текста (она всегда должна быть UTF-8)
* какие символы считаются буквами, а какие нет
* какие буквы следует приводить к другим буквам

Вы можете настроить эти параметры для каждой таблицы отдельно с помощью опции [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). `charset_table` задаёт массив, который сопоставляет буквенные символы с их приведёнными к нижнему регистру версиями (или с любыми другими символами, если вам так нужно). Символы, которых нет в массиве, считаются не буквами и будут обрабатываться как разделители слов при индексации или поиске в этой таблице.

Набор символов по умолчанию — `non_cont`, который включает [большинство языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md).

Вы также можете задавать правила замены текстовых шаблонов. Например, при следующих правилах:

```ini
regexp_filter = \**(\d+)\" => \1 inch
regexp_filter = (BLUE|RED) => COLOR
```

Текст `RED TUBE 5" LONG` будет проиндексирован как `COLOR TUBE 5 INCH LONG`, а `PLANK 2" x 4"` будет проиндексирован как `PLANK 2 INCH x 4 INCH`. Эти правила применяются в указанном порядке. Они также применяются к запросам, поэтому поиск `BLUE TUBE` фактически будет искать `COLOR TUBE`.

Подробнее о [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) можно узнать здесь.

## Параметры конфигурации индекса

### charset_table

```ini
# default
charset_table = non_cont

# only English and Russian letters
charset_table = 0..9, A..Z->a..z, _, a..z, \
U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

# english charset defined with alias
charset_table = 0..9, english, _

# override the default transliteration to preserve German umlauts and sharp s;
# uppercase variants are mapped to lowercase
charset_table = non_cont, german
```

<!-- example charset_table -->
`charset_table` задаёт массив, который сопоставляет буквенные символы с их приведёнными к нижнему регистру версиями (или с любыми другими символами, если вам так нужно). Набор символов по умолчанию — `non_cont`, который включает большинство языков с [непрерывным](https://en.wikipedia.org/wiki/Scriptio_continua) письмом.

`charset_table` — ключевой элемент процесса токенизации Manticore, который извлекает ключевые слова из текста документа или текста запроса. Он определяет, какие символы считаются допустимыми, и как они должны преобразовываться (например, нужно ли приводить регистр или нет).

По умолчанию каждому символу сопоставляется 0, что означает, что он не считается допустимым ключевым словом и обрабатывается как разделитель. Как только символ упоминается в таблице, ему сопоставляется другой символ (чаще всего сам символ или строчная буква), и он считается допустимой частью ключевого слова.

`charset_table` использует список сопоставлений, разделённых запятыми, чтобы объявлять символы допустимыми или сопоставлять их с другими символами. Для диапазонов символов доступны сокращённые записи:

* Сопоставление одного символа: `A->a`. Объявляет исходный символ 'A' допустимым внутри ключевых слов и сопоставляет его с целевым символом 'a' (но не объявляет допустимым 'a').
* Сопоставление диапазона: `A..Z->a..z`. Объявляет все символы в исходном диапазоне допустимыми и сопоставляет их с целевым диапазоном. Не объявляет целевой диапазон допустимым. Проверяет длину обоих диапазонов.
* Сопоставление отдельного символа: `a`. Объявляет символ допустимым и сопоставляет его сам с собой. Эквивалентно сопоставлению одного символа `a->a`.
* Сопоставление отдельного диапазона: `a..z`. Объявляет все символы в диапазоне допустимыми и сопоставляет их сами с собой. Эквивалентно диапазонному сопоставлению `a..z->a..z`.
* Шахматное сопоставление диапазона: `A..Z/2`. Сопоставляет каждую пару символов со вторым символом. Например, `A..Z/2` эквивалентно `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`. Это сокращение полезно для блоков Unicode, где заглавные и строчные буквы идут вперемешку.

Для символов с кодами от 0 до 32, а также для символов в диапазоне от 127 до 8-битного ASCII и Unicode, Manticore всегда считает их разделителями. Чтобы избежать проблем с кодировкой конфигурационного файла, 8-битные ASCII-символы и символы Unicode необходимо задавать в форме `U+XXX`, где `XXX` — шестнадцатеричный код точки. Минимально допустимый код символа Unicode — `U+0021`.

Если стандартных сопоставлений недостаточно, вы можете переопределить сопоставления символов, задав их ещё раз в другом правиле. Например, если встроенный массив `non_cont` включает символы `Ä` и `ä` и сопоставляет их оба с ASCII-символом `a`, вы можете переопределить эти символы, добавив для них кодовые точки Unicode, вот так:

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
Помимо описаний символов и сопоставлений, доступны несколько встроенных псевдонимов. Текущие псевдонимы:
* `chinese`
* `cjk`
* `cont`
* `english`
* `german`
* `japanese`
* `korean`
* `non_cont` (`non_cjk`)
* `russian`
* `thai`

Псевдоним `german` сохраняет `ä`, `ö`, `ü` и `ß` вместо сопоставления их с ASCII-символами и приводит их заглавные варианты к строчным, включая `ẞ` в `ß`. Добавьте его после `non_cont`, как показано выше, чтобы сохранить остальные стандартные сопоставления символов.

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

Если вам нужно поддерживать в поиске разные языки, определение наборов допустимых символов и правил приведения для каждого из них может быть трудоёмким. Мы упростили это, предоставив наборы символов по умолчанию `non_cont` и `cont`, которые покрывают языки с непрерывным и непрерывно-разделяемым письмом (китайский, японский, корейский, тайский) соответственно. В большинстве случаев этих наборов символов должно быть достаточно.

Обратите внимание, что следующие языки сейчас **не** поддерживаются:
* Ассамский
* Бишнуприя
* Бухид
* Гаро
* Хмонг
* Хо
* Коми
* Большой Цветочный мяо
* Маба
* Майтхили
* Маратхи
* Менде
* Мру
* Мьене
* Нгамбай
* Одиа
* Сантали
* Синдхи
* Силхетский

Все остальные языки, перечисленные в [списке языков Unicode
list](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/), поддерживаются по умолчанию.

<!-- example charset_table 3 -->
Чтобы работать и с cont-, и с non-cont-языками, задайте параметры в файле конфигурации, как показано ниже (с [исключением](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md) для китайского языка):

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

Если вам не нужна поддержка языков с непрерывным письмом, вы можете просто исключить опции [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) и [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars).
Для получения дополнительной информации об этих параметрах см. соответствующие разделы документации.

Чтобы сопоставлять один символ с несколькими или наоборот, можно использовать [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter).

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
Список смешиваемых символов. Необязательный, по умолчанию пуст.

Смешиваемые символы индексируются и как разделители, и как допустимые символы. Например, если `&` определён как смешиваемый символ и `AT&T` встречается в индексируемом документе, будут проиндексированы три разных ключевых слова: `at&t`, `at` и `t`.

Кроме того, смешиваемые символы могут влиять на индексацию так, что ключевые слова индексируются так, будто смешиваемые символы вообще не были набраны. Это поведение особенно заметно, когда указано `blend_mode = trim_all`. Например, фраза `some_thing` при `blend_mode = trim_all` будет проиндексирована как `some`, `something` и `thing`.

При использовании смешиваемых символов нужно быть осторожным, потому что объявление символа смешиваемым означает, что он больше не является разделителем.
* Поэтому, если вы добавите запятую в `blend_chars` и выполните поиск `dog,cat`, она будет воспринята как один токен `dog,cat`. Если `dog,cat` **не** был проиндексирован как `dog,cat`, а был оставлен только как `dog cat`, совпадения не будет.
* Следовательно, такое поведение следует контролировать настройкой [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode).

Позиции токенов, полученных заменой смешиваемых символов пробелами, назначаются как обычно, а обычные ключевые слова индексируются так, как если бы `blend_chars` вообще не были указаны. Дополнительный токен, который объединяет смешиваемые и несмешиваемые символы, будет помещён в начальную позицию. Например, если в самом начале текстового поля встречается `AT&T company`, то `at` получит позицию 1, `t` - позицию 2, `company` - позицию 3, а `AT&T` тоже получит позицию 1, смешавшись с первым обычным ключевым словом. В результате запросы `AT&T` или просто `AT` будут совпадать с этим документом. Фразовый запрос `"AT T"` тоже совпадёт, как и фразовый запрос `"AT&T company"`.

Смешиваемые символы могут пересекаться со специальными символами, используемыми в синтаксисе запросов, например в `T-Mobile` или `@twitter`. По возможности парсер запроса будет обрабатывать смешиваемый символ как смешиваемый. Например, если `hello @twitter` находится в кавычках (оператор фразы), парсер запроса будет обрабатывать символ `@` как смешиваемый. Однако если символ `@` не находится в кавычках, он будет обработан как оператор. Поэтому рекомендуется экранировать ключевые слова.

Смешиваемые символы можно переназначать так, чтобы несколько разных смешиваемых символов нормализовались в одну базовую форму. Это полезно при индексировании нескольких альтернативных кодовых точек Unicode с эквивалентными глифами.

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
Режим индексирования смешанных токенов включается директивой `blend_mode`.

По умолчанию токены, в которых смешиваются смешиваемые и несмешиваемые символы, индексируются целиком. Например, если и символ `@`, и восклицательный знак входят в `blend_chars`, строка `@dude!` будет проиндексирована как два токена: `@dude!` (со всеми смешиваемыми символами) и `dude` (без них). В результате запрос `@dude` не будет с ним совпадать.

`blend_mode` добавляет гибкость этому поведению индексации. Он принимает список опций, разделённых запятыми, и каждая из них задаёт вариант индексирования токена.

Если указано несколько опций, будут проиндексированы несколько вариантов одного и того же токена. Обычные ключевые слова (полученные из этого токена заменой смешиваемых символов разделителем) всегда индексируются.

Варианты такие:

* `trim_none` - индексировать весь токен
* `trim_head` - обрезать смешиваемые символы в начале и индексировать получившийся токен
* `trim_tail` - обрезать смешиваемые символы в конце и индексировать получившийся токен
* `trim_both`- обрезать смешиваемые символы и в начале, и в конце и индексировать получившийся токен
* `trim_all` - обрезать смешиваемые символы в начале, в конце и в середине и индексировать получившийся токен
* `skip_pure` - не индексировать токен, если он полностью состоит из смешиваемых символов

Если использовать `blend_mode` с примером строки `@dude!` выше, настройка `blend_mode = trim_head, trim_tail` приведёт к двум проиндексированным токенам: `@dude` и `dude!`. Использование `trim_both` не даст эффекта, потому что при обрезке обоих смешиваемых символов получится `dude`, а он уже индексируется как обычное ключевое слово. Индексирование `@U.S.A.` с `trim_both` (и при условии, что точка тоже смешиваемый символ) приведёт к индексации `U.S.A`. Наконец, `skip_pure` позволяет игнорировать только последовательности смешиваемых символов. Например, `one @@@ two` будет проиндексировано как `one two`, и это будет найдено как фраза. По умолчанию это не так, потому что полностью смешанный токен индексируется и смещает вторую позицию ключевого слова.

Поведение по умолчанию - индексировать весь токен, что эквивалентно `blend_mode = trim_none`.

Имейте в виду, что использование режимов смешивания ограничивает поиск, даже в режиме по умолчанию `trim_none`, если вы считаете `.` смешиваемым символом:
* `.dog.` при индексации станет `.dog. dog`
* и вы не сможете найти его по `dog.`.

Использование большего числа режимов повышает шанс, что ваше ключевое слово что-то совпадёт.

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

`min_word_len` — необязательный параметр конфигурации индекса в Manticore, который задаёт минимальную длину индексируемого слова. Значение по умолчанию — 1, что означает, что индексируется всё.

Будут индексироваться только те слова, длина которых не меньше этого минимума. Например, если `min_word_len` равен 4, то 'the' не будет проиндексировано, а 'they' будет.

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
Длины N-грамм для индексирования N-грамм. Необязательный, по умолчанию 0 (индексирование N-грамм отключено). Известные значения: 0 и 1.

N-граммы обеспечивают базовую поддержку языков с непрерывным письмом в неразделённых текстах. Проблема поиска в таких языках в том, что между словами нет чётких разделителей. В некоторых случаях может не понадобиться сегментация на основе словаря, например [та, что доступна для китайского языка](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md). В таких случаях сегментация на N-граммы тоже может хорошо работать.

Когда эта функция включена, последовательности таких языков (или любых других символов, заданных в [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)) индексируются как N-граммы. Например, если входной текст — "ABCDEF" (где A to F обозначают некоторые символы языка) и `ngram_len` равен 1, он будет проиндексирован так, как будто это "A B C D E F". Сейчас поддерживается только `ngram_len=1`. Только те символы, которые перечислены в таблице [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars), будут разделены таким образом; остальные затронуты не будут.

Обратите внимание: если поисковый запрос уже сегментирован, то есть между отдельными словами есть разделители, то заключение слов в кавычки и использование расширенного режима приведёт к корректному совпадению, даже если текст **не** был сегментирован. Например, если исходный запрос — `BC DEF`, после обрамления кавычками на стороне приложения он должен выглядеть как `"BC" "DEF"` (*с* кавычками). Этот запрос будет передан в Manticore и внутренне тоже разобьётся на 1-граммы, в результате чего получится запрос `"B C" "D E F"`, всё ещё с кавычками, которые являются оператором поиска фразы. И он совпадёт с текстом, даже если в тексте не было разделителей.

Даже если поисковый запрос не сегментирован, Manticore всё равно должна выдавать хорошие результаты благодаря ранжированию по фразам: она будет поднимать выше более близкие фразовые совпадения (что в случае N-грамм может означать более близкие многосимвольные совпадения).

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
Список символов N-грамм. Необязательный, по умолчанию пуст.

Используется вместе с [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) и задаёт символы, последовательности которых подлежат извлечению N-грамм. Слова, состоящие из других символов, не будут затронуты функцией индексирования N-грамм. Формат значения идентичен [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Символы N-грамм не могут присутствовать в [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table).

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
Также вы можете использовать псевдоним для нашей таблицы N-грамм по умолчанию, как в примере. В большинстве случаев этого должно быть достаточно.

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
Список игнорируемых символов. Необязательный, по умолчанию пуст.

Полезно в случаях, когда некоторые символы, например мягкий знак переноса (U+00AD), должны не просто считаться разделителями, а полностью игнорироваться. Например, если '-' просто отсутствует в `charset_table`, текст "abc-def" будет проиндексирован как ключевые слова "abc" и "def". Напротив, если '-' добавить в список `ignore_chars`, тот же текст будет проиндексирован как одно ключевое слово "abcdef".

Синтаксис такой же, как у [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), но разрешено только объявлять символы, а не сопоставлять их. Кроме того, игнорируемые символы не должны присутствовать в `charset_table`.

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
bigram_index = {none|all|first_freq|both_freq|second_numeric|second_has_digit}
```

<!-- example bigram_index -->
Режим индексирования биграмм. Необязательный, по умолчанию none.

Индексирование биграмм - это функция ускорения фразового поиска. При индексации она сохраняет список документов для всех или некоторых пар соседних слов в индексе. Такой список затем можно использовать во время поиска, чтобы значительно ускорить совпадение фраз или подфраз.

`bigram_index` управляет выбором конкретных пар слов. Известные режимы:

* `all`, индексировать каждую пару слов
* `first_freq`, индексировать только те пары слов, где *первое* слово входит в список частотных слов (см. [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)). Например, при `bigram_freq_words = the, in, i, a` индексация текста "alone in the dark" приведёт к сохранению пар "in the" и "the dark" как биграмм, потому что они начинаются с частотного ключевого слова (соответственно "in" и "the"), но "alone in" **не** будет проиндексировано, потому что "in" - это *второе* слово в этой паре.
* `both_freq`, индексировать только те пары слов, где оба слова частотные. Продолжая тот же пример, в этом режиме при индексации "alone in the dark" как биграмма будет сохранена только "in the" (самая слабая из них с точки зрения поиска), а остальные пары слов - нет.
* `second_numeric`, индексировать только те пары слов, где *второй* токен состоит только из ASCII-цифр. Например, `xt 806` совпадает, а `xt rt9600` и `xt v2` - нет.
* `second_has_digit`, индексировать только те пары слов, где *второй* токен содержит хотя бы одну ASCII-цифру. Например, `xt 806`, `xt rt9600` и `xt v2` совпадают, а `xt abc` - нет.

Для большинства сценариев `both_freq` будет лучшим режимом, но результат может отличаться в зависимости от задачи.

Важно отметить, что `bigram_index` работает только на уровне токенизации и не учитывает преобразования вроде `morphology`, `wordforms` или `stopwords`. Это означает, что создаваемые им токены очень прямолинейны, что делает поиск фраз более точным и строгим. Хотя это может повысить точность фразового совпадения, система также становится менее способной распознавать разные формы слов или варианты их написания.

Числовые режимы используют только ASCII-цифры (`0-9`). Они не считают `+`, `-` или цифры Unicode числовыми. Проверки также используют текст токена, полученный текущим путём токенизации, без какой-либо дополнительной нормализации знаков препинания.

Используйте [bigram_delimiter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_delimiter), чтобы управлять тем, хранится ли допустимая биграмма как внутренний токен с разделителем, как склеенный токен вроде `iphone17` или в обоих видах.

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

### bigram_delimiter

```ini
bigram_delimiter = {true|none|both}
```

<!-- example bigram_delimiter -->
Режим хранения биграммных токенов. Необязательный, по умолчанию `true`.

`bigram_delimiter` управляет тем, какая форма токена сохраняется для допустимых биграмм, выбранных [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index):

* `true`, сохранять только внутренний биграммный токен с разделителем. Это текущее поведение по умолчанию.
* `none`, сохранять только склеенную форму токена, например `iphone17`.
* `both`, сохранять и внутреннюю форму с разделителем, и склеенную форму.

Поведение поиска зависит от выбранного режима:

* при `true` оптимизация фраз переписывает допустимые пары фраз во внутренний токен с разделителем
* при `none` оптимизация фраз переписывает допустимые пары фраз в склеенный токен, например `"iphone 17"` превращается в `iphone17`
* при `both` оптимизация фраз пропускается, и фразовые запросы остаются обычными фразовыми запросами, при этом поиск по склеенному токену всё равно может совпасть, потому что склеенная форма тоже хранится

`bigram_delimiter` меняет только форму хранимого токена. Он не определяет, какие пары допустимы; это по-прежнему задаётся [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_index = 'all' bigram_delimiter = 'none'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) bigram_index = 'all' bigram_delimiter = 'none'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_index' => 'all',
            'bigram_delimiter' => 'none'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_index = \'all\' bigram_delimiter = \'none\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_index = \'all\' bigram_delimiter = \'none\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_index = \'all\' bigram_delimiter = \'none\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_index = 'all' bigram_delimiter = 'none'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_index = 'all' bigram_delimiter = 'none'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) bigram_index = 'all' bigram_delimiter = 'none'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  bigram_index = all
  bigram_delimiter = none

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
Список ключевых слов, считающихся "частотными" при индексировании биграмм. Необязательный, по умолчанию пуст.

Некоторые режимы индексирования биграмм (см. [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)) требуют списка частотных ключевых слов. Их **не** следует путать со стоп-словами. Стоп-слова полностью удаляются и при индексации, и при поиске. Частотные ключевые слова используются только биграммами, чтобы определить, следует индексировать текущую пару слов или нет.

`bigram_freq_words` позволяет задать такой список ключевых слов.

Эта опция нужна только для `first_freq` и `both_freq`.

Она должна оставаться пустой для:

* `none`
* `all`
* `second_numeric`
* `second_has_digit`

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
dict = {keywords|keywords_32k|crc}
```

<!-- example dict -->
Тип словаря определяется одним из трёх известных значений: `keywords`, `keywords_32k` или `crc`. Эта настройка необязательна; по умолчанию используется `keywords`.

`dict=keywords` и `dict=keywords_32k` - это словарные режимы. Словарный режим хранит исходный текст ключевого слова в индексе и выполняет расширение подстановочных шаблонов во время поиска. `dict=crc` вместо этого хранит контрольные суммы ключевых слов.

`dict=keywords` - это словарный режим по умолчанию.

`dict=keywords_32k` - это включаемый режим словаря для токенов ключевых слов размером до 32 КиБ. Он поддерживает нормализованные токены размером до 32768 байт как в обычных, так и в RT-таблицах. Токены, превышающие этот предел, пропускаются с предупреждением вместо того, чтобы индексироваться как обрезанные термины. Поддерживаются точный поиск, поиск по префиксу и поиск по инфиксу, если включены обычные настройки точного, префиксного или инфиксного поиска. Подробности о лимите обычного токена в 42 байта и лимите `keywords_32k` в 32768 байт см. в разделе [Ограничение длины токена](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md#Token-length-limit).

`keywords_32k` предназначен для длинных машинно сгенерированных значений, таких как хэши, сгенерированные идентификаторы, идентификаторы сообщений и длинные токены, похожие на адреса электронной почты.

Следующие функции пока не поддерживают `dict=keywords_32k`:

* `CALL SUGGEST` и `CALL QSUGGEST` не работают с таблицами, использующими `dict=keywords_32k`.
* Percolate-таблицы не могут использовать `dict=keywords_32k`.
* Сниппеты и подсветка по-прежнему используют обычный лимит токенов. Токены длиной до 42 байт могут быть подсвечены; более длинные токены `keywords_32k` пропускаются при обработке сниппетов и подсветки.
* `indextool --dumpdict` пока не может выгружать словари `dict=keywords_32k`.

CRC-словарь не хранит исходный текст ключевого слова в индексе. Вместо этого он заменяет ключевые слова значением контрольной суммы (вычисленным с помощью FNV64) как при поиске, так и при индексации. Это значение используется внутри индекса. У такого подхода есть два недостатка:
* Во-первых, существует риск столкновения контрольных сумм у разных пар ключевых слов. Этот риск растёт пропорционально числу уникальных ключевых слов в индексе. Тем не менее это незначительная проблема, поскольку вероятность одного столкновения FNV64 в словаре из 1 миллиарда записей составляет примерно 1 к 16, то есть 6,25%. Большинство словарей будут содержать гораздо меньше миллиарда ключевых слов, учитывая, что в типичном разговорном человеческом языке бывает от 1 до 10 миллионов словоформ.
* Во-вторых, и что важнее, с контрольными суммами не так просто выполнять поиски подстрок. Manticore решила эту проблему, предварительно индексируя все возможные подстроки как отдельные ключевые слова (см. директивы [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len), [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)). У этого метода есть даже дополнительное преимущество: подстроки совпадают максимально быстро. Однако предварительная индексация всех подстрок значительно увеличивает размер индекса (часто в 3-10 раз и более), а затем влияет и на время индексации, делая поиск подстрок по большим индексам довольно непрактичным.

Словарный режим решает обе эти проблемы. Он хранит ключевые слова в индексе и выполняет расширение подстановочных шаблонов во время поиска. Например, поиск префикса `test*` может внутренне расшириться в запрос `'test|tests|testing'` на основе содержимого словаря. Этот процесс расширения полностью прозрачен для приложения, за исключением того, что теперь также возвращается отдельная статистика по каждому совпавшему ключевому слову.

Для поиска подстрок (инфиксов) можно использовать расширенные подстановки. Специальные символы, такие как `?` и `%`, совместимы с поиском подстрок (инфиксов) (например, `t?st*`, `run%`, `*abc*`). Обратите внимание, что [операторы подстановок](../../Searching/Full_text_matching/Operators.md#Wildcard-operators) работают с `dict=keywords` и `dict=keywords_32k`, тогда как оператор [REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) работает только с `dict=keywords`.

Для токенов обычного размера индексирование со словарным режимом примерно в 1,1-1,3 раза медленнее, чем обычная индексация без подстрок, но значительно быстрее, чем индексирование подстрок (как префиксное, так и инфиксное). Размер индекса должен быть лишь немного больше, чем у стандартной таблицы без подстрок, с общей разницей в 1..10%. Время обычного поиска по ключевым словам должно быть почти одинаковым или идентичным для всех трёх рассматриваемых типов индекса (CRC без подстрок, CRC с подстроками, словарный режим). Время поиска подстрок может значительно меняться в зависимости от того, сколько реальных ключевых слов совпадает с данной подстрокой (то есть во сколько ключевых слов расширяется поисковый термин). Максимальное число совпавших ключевых слов ограничено директивой [expansion_limit](../../Server_settings/Searchd.md#expansion_limit).

Итог такой: словарный и CRC-режимы предлагают два разных компромисса для поиска подстрок. Можно пожертвовать временем индексации и размером индекса, чтобы получить максимально быстрый поиск в худшем случае (CRC-словарь), либо минимально влиять на время индексации, но пожертвовать временем поиска в худшем случае, когда префикс расширяется в большое число ключевых слов (словарйный режим).

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
Ограничение размера встроенных файлов exceptions, wordforms или stop words. Необязательный, по умолчанию 16K.

При создании таблицы указанные выше файлы можно либо сохранить отдельно вместе с таблицей, либо встроить непосредственно в таблицу. Файлы размером меньше `embedded_limit` сохраняются в таблице. Для более крупных файлов сохраняются только имена файлов. Это также упрощает перенос файлов таблицы на другой сервер: иногда достаточно скопировать один файл.

Для небольших файлов такое встраивание уменьшает число внешних файлов, от которых зависит таблица, и упрощает сопровождение. Но в то же время нет смысла встраивать словарь `wordforms` размером 100 МБ в маленькую delta-таблицу. Поэтому нужен порог по размеру, и `embedded_limit` - это как раз такой порог.

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
Путь к файлу с глобальными (кластерными) IDF ключевых слов. Необязательный, по умолчанию пуст (используются локальные IDF).

В кластере из нескольких таблиц частоты по ключевым словам, скорее всего, будут различаться между таблицами. Это означает, что когда функция ранжирования использует значения на основе TF-IDF, такие как факторы семейства BM25, результаты могут ранжироваться немного по-разному в зависимости от узла кластера, на котором они находятся.

Самый простой способ исправить эту проблему - создать и использовать глобальный словарь частот, или сокращённо глобальный IDF-файл. Эта директива позволяет указать расположение такого файла. Рекомендуется, но не обязательно, использовать расширение .idf. Когда IDF-файл указан для данной таблицы и [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) установлен в 1, движок будет использовать частоты ключевых слов и количество документов коллекции из global_idf-файла, а не только из локальной таблицы. Так IDF и зависящие от них значения будут оставаться согласованными во всём кластере.

IDF-файлы можно использовать совместно для нескольких таблиц. `searchd` загрузит только одну копию IDF-файла, даже если на него ссылается множество таблиц. Если содержимое IDF-файла изменится, новые данные можно загрузить с помощью SIGHUP.

Вы можете собрать .idf-файл с помощью утилиты [indextool](../../Miscellaneous_tools.md#indextool), сначала выгрузив словари с помощью ключа `--dumpdict dict.txt --stats`, затем преобразовав их в формат .idf с помощью `--buildidf`, а потом объединив все .idf-файлы по кластеру с помощью `--mergeidf`.

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
Список hitless-слов. Необязательный, допустимые значения: 'all' или имя файла со списком.

По умолчанию полнотекстовый индекс Manticore хранит не только список совпадающих документов для каждого заданного ключевого слова, но и список его позиций внутри документа (так называемый hitlist). Hitlist'ы позволяют выполнять поиск по фразам, по близости, в строгом порядке и другие расширенные типы поиска, а также ранжирование по близости фраз. Однако hitlist'ы для отдельных частотных ключевых слов (которые по какой-то причине нельзя исключить, несмотря на их частоту) могут стать очень большими и, следовательно, медленными в обработке при поиске. Кроме того, в некоторых случаях нам может быть нужен только булев поиск по ключевым словам, и тогда позиционные операторы поиска (такие как поиск по фразе) или ранжирование по фразам вообще не нужны.

`hitless_words` позволяет создавать индексы, которые либо вообще не содержат позиционной информации (hitlist'ов), либо не хранят её для отдельных ключевых слов.

Hitless-индекс обычно занимает меньше места, чем соответствующий обычный полнотекстовый индекс (можно ожидать примерно в 1,5 раза меньший размер). И индексация, и поиск должны быть быстрее, но ценой отсутствия поддержки позиционных запросов и ранжирования.

Если такие слова используются в позиционных запросах (например, в запросах по фразам), они удаляются из них и используются как операнд без позиции. Например, если "hello" и "world" являются hitless, а "simon" и "says" - нет, то фразовый запрос `"simon says hello world"` будет преобразован в `("simon says" & hello & world)`, где "hello" и "world" будут совпадать в любом месте документа, а "simon says" - как точная фраза.

Если позиционный запрос содержит только hitless-слова, в результате получится пустой узел фразы, поэтому весь запрос вернёт пустой результат и предупреждение. Если весь словарь hitless (используется `all`), на соответствующем индексе можно использовать только булев поиск.



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
Параметр `hitless_words_list` позволяет указать hitless-слова прямо в операторе `CREATE TABLE`. Он поддерживается только в [RT-режиме](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29).

Значения должны быть разделены точками с запятой (`;`).

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
Включает вычисление и сохранение длин полей (как по документам, так и средних значений по индексу) в полнотекстовом индексе. Необязательный, по умолчанию 0 (не вычислять и не сохранять).

Когда `index_field_lengths` установлен в 1, Manticore будет:
* создавать соответствующий атрибут длины для каждого полнотекстового поля с тем же именем, но с суффиксом `__len`
* вычислять длину поля (считая в ключевых словах) для каждого документа и сохранять её в соответствующий атрибут
* вычислять средние значения по индексу. Атрибуты длины будут иметь специальный тип TOKENCOUNT, но на самом деле их значения - обычные 32-битные целые числа, и к ним обычно можно обращаться напрямую.

Функции [BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) и [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) в ранжировщике expressions основаны на этих длинах и требуют включённого `index_field_lengths`. Исторически Manticore использовала упрощённый, облегчённый вариант BM25, который, в отличие от полной функции, **не** учитывал длину документа. Также поддерживаются полная версия BM25 и её расширение для нескольких полей, называемое BM25F. Они требуют соответственно длины по документу и длины по полям. Отсюда и дополнительная директива.

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
Фильтр токенов на этапе индексации для полнотекстовой индексации. Необязательный, по умолчанию пуст.

Директива `index_token_filter` задаёт необязательный фильтр токенов на этапе индексации для полнотекстовой индексации. Эта директива используется для создания пользовательского токенизатора, который формирует токены по пользовательским правилам. Фильтр создаётся indexer'ом при индексации исходных данных в обычную таблицу или RT-таблицей при обработке операторов `INSERT` или `REPLACE`. Плагины задаются в формате `library name:plugin name:optional string of settings`. Например, `my_lib.so:custom_blend:chars=@#&`.

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
Приращение позиции для слишком коротких ключевых слов (короче [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)). Необязательный, допустимые значения: 0 и 1, по умолчанию 1.

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
Список символов границ фразы. Необязательный, по умолчанию пуст.

Этот список управляет тем, какие символы будут считаться границами фразы, чтобы корректировать позиции слов и обеспечивать эмуляцию поиска на уровне фраз через поиск по близости. Синтаксис похож на [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), но сопоставления не допускаются, а символы границ не должны пересекаться с чем-либо ещё.

На границе фразы к текущей позиции слова будет добавлено дополнительное приращение позиции (указывается параметром [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)). Это позволяет выполнять поиск на уровне фраз через запросы по близости: слова из разных фраз гарантированно будут находиться друг от друга на расстоянии больше phrase_boundary_step; поэтому поиск по близости в пределах этого расстояния будет эквивалентен поиску по фразе.

Условие границы фразы будет срабатывать тогда и только тогда, когда за таким символом следует разделитель; это нужно, чтобы сокращения вроде S.T.A.L.K.E.R или URL не воспринимались как несколько фраз.

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
Приращение позиции слова на границе фразы. Необязательный, по умолчанию 0.

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
Регулярные выражения (regexps), используемые для фильтрации полей и запросов. Эта директива необязательна, поддерживает несколько значений, а по умолчанию представляет собой пустой список регулярных выражений. Движок регулярных выражений, используемый Manticore Search, - это Google RE2, известный своей скоростью и безопасностью. Подробную информацию о синтаксисе, поддерживаемом RE2, можно найти в [руководстве по синтаксису RE2](https://github.com/google/re2/wiki/Syntax).

В некоторых сценариях, например при поиске товаров, один и тот же продукт, модель или свойство можно называть по-разному. Например, `iPhone 3gs` и `iPhone 3 gs` (или даже `iPhone3 gs`) очень вероятно относятся к одному и тому же продукту. Другой пример - разные способы указать размер экрана ноутбука, такие как `13-inch`, `13 inch`, `13"` или `13in`.

Регулярные выражения позволяют задавать правила, специально рассчитанные на такие случаи. В первом примере можно было бы использовать файл wordforms, чтобы обработать несколько моделей iPhone, но во втором лучше задать правила, которые нормализуют "13-inch" и "13in" к одному и тому же виду.

Регулярные выражения, перечисленные в `regexp_filter`, применяются в том порядке, в котором они указаны, на самой ранней возможной стадии, до любой другой обработки (включая [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)), даже до токенизации. То есть регулярные выражения применяются к сырым исходным полям при индексации и к сырую тексту поискового запроса при поиске.

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
