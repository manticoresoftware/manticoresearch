# Низкоуровневая токенизация  

Когда текст индексируется в Manticore, он разбивается на слова, и делается приведение регистра, так что слова вроде "Abc", "ABC" и "abc" рассматриваются как одинаковое слово.

Чтобы корректно выполнять эти операции, Manticore должен знать:
* кодировку исходного текста (которая всегда должна быть UTF-8)
* какие символы считаются буквами, а какие - нет
* какие буквы должны быть приведены к другим буквам

Вы можете настроить эти параметры для каждой таблицы с помощью опции  [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). charset_table задает массив, который связывает буквы с их приведенными к нижнему регистру версиями (или любыми другими символами, которые вы предпочитаете). Символы, отсутствующие в массиве, считаются небуквами и будут рассматриваться как разделители слов во время индексации или поиска в этой таблице.

Набор символов по умолчанию - это `non_cont`, который включает в себя [большинство языков](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md).

Вы также можете определить правила замены текстовых шаблонов. Например, с помощью следующих правил:

```ini
regexp_filter = \**(\d+)\" => \1 дюйм
regexp_filter = (BLUE|RED) => ЦВЕТ
```

Текст `RED TUBE 5" LONG` будет индексирован как `ЦВЕТ TUBE 5 ДЮЙМ LONG`, а `PLANK 2" x 4"` будет индексирован как `PLANK 2 ДЮЙМ x 4 ДЮЙМ`. Эти правила применяются в указанном порядке. Правила также применяются к запросам, поэтому поиск `BLUE TUBE` на самом деле будет искать `ЦВЕТ TUBE`.

Вы можете узнать больше о [regexp_filter здесь](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter).

## Параметры конфигурации индекса 

### charset_table

```ini
# по умолчанию
charset_table = non_cont

# только английские и русские буквы
charset_table = 0..9, A..Z->a..z, _, a..z, \
U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

# английская кодировка, определенная с помощью псевдонима
charset_table = 0..9, english, _

# вы можете переопределить соответствия символов, определив их заново, например, для нечувствительного к регистру поиска с немецкими умлаутами вы можете использовать:
charset_table = non_cont, U+00E4, U+00C4->U+00E4, U+00F6, U+00D6->U+00F6, U+00FC, U+00DC->U+00FC, U+00DF, U+1E9E->U+00DF
```

<!-- пример charset_table -->
`charset_table` задает массив, который связывает буквы с их приведенными к нижнему регистру версиями (или любыми другими символами, если вы предпочитаете). Набор символов по умолчанию - это `non_cont`, который включает в себя большинство языков с [неконтинуальными](https://en.wikipedia.org/wiki/Scriptio_continua) письмами.

`charset_table` является основной частью процесса токенизации Manticore, который извлекает ключевые слова из текстов документов или текстов запросов. Он контролирует, какие символы принимаются как допустимые и как они должны быть преобразованы (например, следует ли удалять регистр или нет).

По умолчанию каждый символ отображается как 0, что означает, что он не считается допустимым ключевым словом и обрабатывается как разделитель. Как только символ упоминается в таблице, он отображается на другой символ (чаще всего либо на себя, либо на строчную букву) и обрабатывается как часть допустимого ключевого слова.

charset_table использует запятую для разделения списков соответствий, чтобы объявить символы допустимыми или сопоставить их с другими символами. Синтаксические сокращения доступны для сопоставления диапазонов символов сразу:

* Соответствие одного символа: `A->a`. Объявляет исходный символ 'A' как допустимый в ключевых словах и сопоставляет его с целевым символом 'a' (но не объявляет 'a' как допустимый).
* Сопоставление диапазона: `A..Z->a..z`. Объявляет все символы в исходном диапазоне как допустимые и сопоставляет их с целевым диапазоном. Не объявляет целевой диапазон как допустимый. Проверяет длины обоих диапазонов.
* Сопоставление одиночного символа: `a`. Объявляет символ как допустимый и сопоставляет его с самим собой. Эквивалентно `a->a` сопоставлению одного символа.
* Сопоставление диапазона одиночных символов: `a..z`.  Объявляет все символы в диапазоне как допустимые и сопоставляет их с собой. Эквивалентно `a..z->a..z` сопоставлению диапазона.
* Сопоставление шахматной доски: `A..Z/2`. Сопоставляет каждую пару символов на второй символ. Например, `A..Z/2` эквивалентно `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z`. Это синтаксическое сокращение полезно для блоков Unicode, где прописные и строчные буквы идут в чередующемся порядке.

Для символов с кодами от 0 до 32, а также для символов в диапазоне от 127 до 8-битного ASCII и символов Unicode, Manticore всегда рассматривает их как разделители. Чтобы избежать проблем с кодировкой файлов конфигурации, 8-битные символы ASCII и символы Unicode должны быть указаны в виде `U+XXX`, где `XXX` - это шестнадцатеричный номер кода. Минимально допустимый код символа Unicode - это `U+0021`.

Если стандартные соответствия недостаточны для ваших нужд, вы можете переопределить соответствия символов, указав их снова с помощью другого соответствия. Например, если встроенный массив `non_cont` включает символы `Ä` и `ä` и сопоставляет их обоих с ASCII символом `a`, вы можете переопределить эти символы, добавив их кодовые точки Unicode, как это:

```
charset_table = non_cont,U+00E4,U+00C4
```

для поиска с учетом регистра или

```
charset_table = non_cont,U+00E4,U+00C4->U+00E4
```

для нечувствительного к регистру поиска.

<!-- запрос SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
```

<!-- запрос JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'"
```

<!-- запрос PHP -->

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'");
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
Помимо определений символов и преобразований, существует несколько встроенных псевдонимов, которые можно использовать. Текущие псевдонимы:
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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'");
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

Если вы хотите поддерживать разные языки в вашем поиске, определение наборов допустимых символов и правил преобразования для всех них может быть трудоемкой задачей. Мы упростили это для вас, предоставив таблицы символов по умолчанию, `non_cont` и `cont`, которые охватывают языки с непрерывными и непрерывными (китайский, японский, корейский, тайский) скриптами соответственно. В большинстве случаев этих наборов символов должно быть достаточно для ваших нужд.

Обратите внимание, что следующие языки в настоящее время **не** поддерживаются:
* Ассамский
* Бишнуприя
* Бухид
* Гаро
* Хмонг
* Хо
* Коми
* Большой цветочный мяо
* Маба
* Майтхили
* Маратхи
* Менде
* Мру
* Мьене
* Нгамбай
* Одия
* Сантали
* Синдхи
* Силхети

Все остальные языки, перечисленные в [списке языков Unicode
list](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/), поддерживаются по умолчанию.

<!-- example charset_table 3 -->
Для работы как с непрерывными, так и с прерывными языками, установите параметры в вашем конфигурационном файле, как показано ниже (с [исключением](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md) для китайского):

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'");
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

Если вам не требуется поддержка языков с непрерывным письмом, вы можете просто исключить [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) и [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars).
options. Для получения дополнительной информации об этих параметрах обратитесь к соответствующим разделам документации.

Чтобы сопоставить один символ с несколькими символами или наоборот, вы можете использовать [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter), это может быть полезно.

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
Список смешанных символов. Необязательно, по умолчанию пустой.

Смешанные символы индексируются как разделители и допустимые символы. Например, когда `&` определен как смешанный символ и `AT&T` появляется в индексированном документе, будут проиндексированы три различных ключевых слова: `at&t`, `at` и `t`.

Кроме того, смешанные символы могут влиять на индексацию таким образом, что ключевые слова индексируются так, как если бы смешанные символы вообще не были введены. Это поведение особенно явно, когда указано `blend_mode = trim_all`. Например, фраза `some_thing` будет проиндексирована как `some`, `something` и `thing` с `blend_mode = trim_all`.

Следует быть осторожным при использовании смешанных символов, так как определение символа как смешанного означает, что он больше не является разделителем.
* Поэтому, если вы добавите запятую к `blend_chars` и выполните поиск по запросу `dog,cat`, это будет трактоваться как единственный токен `dog,cat`. Если `dog,cat` **не** был проиндексирован как `dog,cat`, а остался только как `dog cat`, то он не совпадет.
* Таким образом, это поведение должно контролироваться с помощью настройки [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode).

Позиции токенов, полученные путем замены смешанных символов пробелами, назначаются как обычно, и обычные ключевые слова будут индексироваться так, как если бы не было указано `blend_chars` вообще. Дополнительный токен, который смешивает смешанные и не смешанные символы, будет помещен в стартовую позицию. Например, если `AT&T company` встречается в самом начале текстового поля, `at` будет назначено на позицию 1, `t` на позицию 2, `company` на позицию 3, а `AT&T` также получит позицию 1, смешиваясь с открывающим обычным ключевым словом. В результате запросы на `AT&T` или просто `AT` будут совпадать с этим документом. Фразовый запрос для `"AT T"` также совпадет, так же как и фразовый запрос для `"AT&T company"`.

Смешанные символы могут пересекаться с специальными символами, используемыми в синтаксисе запросов, такими как `T-Mobile` или `@twitter`. Где это возможно, парсер запросов будет обрабатывать смешанный символ как смешанный. Например, если `hello @twitter` находится в кавычках (оператор фразы), парсер запросов будет обрабатывать символ `@` как смешанный. Однако, если символ `@` не был в кавычках, он будет обрабатываться как оператор. Поэтому рекомендуется экранировать ключевые слова.

Смешанные символы могут быть переназначены таким образом, что несколько различных смешанных символов могут быть нормализованы в одну базовую форму. Это полезно при индексации нескольких альтернативных кодовых точек Unicode с эквивалентными глифами.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'");
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

По умолчанию токены, которые смешивают смешанные и не смешанные символы, индексируются полностью. Например, когда и знак @, и восклицательный знак находятся в `blend_chars`, строка `@dude!` будет проиндексирована как два токена: `@dude!` (со всеми смешанными символами) и `dude` (без каких-либо). В результате запрос `@dude` **не** совпадет с ним.

`blend_mode` добавляет гибкость к этому поведению индексации. Он принимает список опций, разделенных запятыми, каждая из которых задает вариант индексации токена.
Если указано несколько опций, будут индексированы несколько вариантов одного и того же токена. Регулярные ключевые слова (результирующие из этого токена путем замены смешанных символов на разделитель) всегда индексируются.

Опции:

* `trim_none` - Индексировать весь токен
* `trim_head` - Удалить начальные смешанные символы и индексировать полученный токен
* `trim_tail` - Удалить конечные смешанные символы и индексировать полученный токен
* `trim_both`- Удалить как начальные, так и конечные смешанные символы и индексировать полученный токен
* `trim_all` - Удалить начальные, конечные и средние смешанные символы и индексировать полученный токен
* `skip_pure` - Не индексировать токен, если он полностью смешан, то есть состоит только из смешанных символов

Используя `blend_mode` с приведенной выше строкой `@dude!`, установка `blend_mode = trim_head, trim_tail` приведет к индексации двух токенов: `@dude` и `dude!`. Использование `trim_both` не даст эффекта, потому что удаление как начальных, так и конечных смешанных символов дает `dude`, который уже индексируется как регулярное ключевое слово. Индексация `@U.S.A.` с `trim_both` (при предположении, что точка смешана дважды) приведет к индексации `U.S.A`. Наконец, `skip_pure` позволяет игнорировать последовательности только смешанных символов. Например, `one @@@ two` будет индексироваться как `one two` и соответствовать этому как фраза. Это не так по умолчанию, потому что полностью смешанный токен индексируется и смещает позицию второго ключевого слова.

Поведение по умолчанию заключается в индексировании всего токена, что эквивалентно `blend_mode = trim_none`.

Имейте в виду, что использование режимов смешивания ограничивает ваш поиск, даже с режимом по умолчанию `trim_none`, если вы предполагаете, что `.` является смешанным символом:
* `.dog.` станет `.dog. dog` при индексации
* и вы не сможете найти его по `dog.`.

Использование большего количества режимов увеличивает вероятность того, что ваше ключевое слово совпадет с чем-то.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'");
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

min_word_len является необязательной опцией конфигурации индекса в Manticore, которая указывает минимальную длину слова для индексации. Значение по умолчанию - 1, что означает, что все индексируется.

Только те слова, которые не короче этого минимального, будут индексироваться. Например, если min_word_len равно 4, тогда 'the' не будет индексироваться, но 'they' будет.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_word_len = '4'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_word_len = '4'");
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
Длина N-грамм для индексации N-грамм. Необязательный, по умолчанию 0 (отключить индексирование N-грамм). Известные значения - 0 и 1.
N-граммы обеспечивают базовую поддержку для языков с непрерывным письмом в неразделенных текстах. Проблема поиска в языках с непрерывным письмом заключается в отсутствии четких разделителей между словами. В некоторых случаях вы можете не захотеть использовать сегментацию на основе словаря, такую как [доступная для китайского языка](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md). В таких случаях сегментация на основе n-граммов также может работать хорошо.

Когда эта функция включена, поток таких языков (или любых других символов, определенных в [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)) индексируется как N-граммы. Например, если входной текст — "ABCDEF" (где A–F представляют собой символы какого-либо языка), а ngram_len равен 1, он будет индексироваться так, как будто он был "A B C D E F". В настоящее время поддерживается только ngram_len=1. Только те символы, которые перечислены в таблице [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars), будут разбиты таким образом; остальные не будут затронуты.

Обратите внимание, что если поисковой запрос сегментирован, т.е. между отдельными словами есть разделители, то обрамление слов в кавычки и использование расширенного режима приведет к тому, что соответствия будут найдены, даже если текст **не** был сегментирован. Например, предположим, что исходный запрос — `BC DEF`. После обрамления в кавычки со стороны приложения, он должен выглядеть как `"BC" "DEF"` (*с* кавычками). Этот запрос будет передан в Manticore и внутренне также разбит на 1-граммы, что приведет к запросу `"B C" "D E F"`, также с кавычками, которые являются оператором совпадения фраз. И он будет соответствовать тексту, хотя в тексте не было разделителей.

Даже если поисковой запрос не сегментирован, Manticore все равно должен выдавать хорошие результаты благодаря ранжированию на основе фраз: он будет поднимать более близкие соответствия фраз (что в случае слов N-грамма может означать более близкие многосимвольные соответствия) к вершине.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
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
Список символов n-грамма. Необязательный, по умолчанию пустой.

Используется в сочетании с [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len), этот список определяет символы, последовательности которых подлежат извлечению n-граммов. Слова, состоящие из других символов, не будут затронуты функцией индексации n-граммов. Формат значения идентичен [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Символы n-грамма не могут появляться в [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table).

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
##### Javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'");
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
Также вы можете использовать псевдоним для таблицы N-грамм по умолчанию, как показано в примере. Это должно быть достаточным в большинстве случаев.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'");
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

Полезен в случаях, когда некоторые символы, такие как мягкий дефис (U+00AD), должны не просто обрабатываться как разделители, но полностью игнорироваться. Например, если '-' просто отсутствует в charset_table, текст "abc-def" будет проиндексирован как ключевые слова "abc" и "def". Напротив, если '-' добавлен в список ignore_chars, тот же текст будет проиндексирован как единое ключевое слово "abcdef".

Синтаксис такой же, как и для [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), но разрешено только объявлять символы, но не допускается их сопоставление. Кроме того, игнорируемые символы не должны присутствовать в charset_table.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'");
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
Режим индексирования биграмм. Необязательно, по умолчанию - none.

Индексирование биграмм - это функция для ускорения поиска фраз. При индексировании он сохраняет список документов для всех или некоторых пар соседних слов в индекс. Такой список затем может быть использован во время поиска для значительного ускорения сопоставления фраз или подфраз.

`bigram_index` контролирует выбор конкретных пар слов. Известные режимы:

* `all`, индексировать каждую пару слов
* `first_freq`, индексировать только пары слов, где *первое* слово находится в списке частых слов (см. [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)). Например, с `bigram_freq_words = the, in, i, a`, индексирование текста "alone in the dark" приведет к сохранению пар "in the" и "the dark" в качестве биграмм, потому что они начинаются с частого ключевого слова (либо "in", либо "the" соответственно), но "alone in" **не** будет проиндексирован, поскольку "in" является *вторым* словом в этой паре.
* `both_freq`, индексировать только пары слов, где оба слова являются частыми. Продолжая тот же пример, в этом режиме индексирование "alone in the dark" сохранит только "in the" (самое плохее с точки зрения поиска) в качестве биграммы, но ни одна другая пара слов сохранена не будет.

Для большинства вариантов использования `both_freq` будет лучшим режимом, но результаты могут варьироваться.

Важно отметить, что `bigram_index` работает только на уровне токенизации и не учитывает преобразования типа `morphology`, `wordforms` или `stopwords`. Это означает, что создаваемые им токены очень просты, что делает поиск фраз более точным и строгим. Хотя это может улучшить точность сопоставления фраз, это также делает систему менее способной распознавать разные формы слов или вариации их написания.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'");
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
Список ключевых слов, которые считаются "частыми" при индексации биграмм. Опционально, значение по умолчанию — пустое.

Некоторые режимы индексации биграмм (см. [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)) требуют определения списка частых ключевых слов. Эти слова **не** следует путать со стоп-словами. Стоп-слова полностью устраняются как при индексации, так и при поиске. Частые ключевые слова используются только биграммами, чтобы определить, индексировать ли текущую пару слов или нет.

`bigram_freq_words` позволяет вам определить список таких ключевых слов.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'");
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

Использование режима словаря ключевых слов (dict=keywords) может значительно снизить нагрузку на индексацию и позволить выполнять поиск по подстрокам в обширных коллекциях. Этот режим можно использовать для обычных и RT таблиц.

CRC-словари не сохраняют оригинальный текст ключевого слова в индексе. Вместо этого они заменяют ключевые слова на значение контрольной суммы (вычисленное с использованием FNV64) как при поиске, так и при индексации. Это значение используется внутри индекса. Этот подход имеет два недостатка:
* Во-первых, существует риск столкновений контрольной суммы между различными парами ключевых слов. Этот риск возрастает пропорционально количеству уникальных ключевых слов в индексе. Тем не менее, эта проблема несущественна, поскольку вероятность одной коллизии FNV64 в словаре из 1 миллиарда записей составляет примерно 1 к 16, или 6.25 процента. Большинство словарей будут содержать гораздо меньше миллиарда ключевых слов, учитывая, что типичный разговорный человеческий язык имеет от 1 до 10 миллионов форм слов.
* Во-вторых, и что более важно, выполнение подстрочных поисков с контрольными суммами не является простым. Manticore решила эту проблему, предварительно индексируя все возможные подстроки как отдельные ключевые слова (см. директивы [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len), [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)). Этот метод даже имеет дополнительное преимущество в том, что он соответствует подстрокам самым быстрым способом. Тем не менее, предварительная индексация всех подстрок значительно увеличивает размер индекса (часто в 3-10 раз и более) и, следовательно, влияет на время индексации, делая подстрочные поиски по крупным индексам довольно непрактичными.

Словарь ключевых слов решает обе эти проблемы. Он хранит ключевые слова в индексе и выполняет расширение подстановочных знаков во время поиска. Например, поиск префикса `test*` может внутренне расшириться в запрос 'test|tests|testing' в зависимости от содержимого словаря. Этот процесс расширения совершенно невидим для приложения, за исключением того, что отдельные статистики по каждому ключевому слову для всех совпадающих ключевых слов теперь также отображаются.

Для подстрочных (инфиксных) поисков можно использовать расширенные подстановочные знаки. Специальные символы, такие как `?` и `%`, совместимы с подстрочным (инфиксным) поиском (например, `t?st*`, `run%`, `*abc*`). Обратите внимание, что [операторы подстановочных знаков](Searching/Full_text_matching/Operators.md#Wildcard-operators) и [REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) работают только с `dict=keywords`.

Индексация с помощью словаря ключевых слов примерно на 1.1x-1.3x медленнее, чем обычная индексация без подстрок, но значительно быстрее, чем подстрочная индексация (префиксная или инфиксная). Размер индекса должен быть лишь немного больше, чем у стандартной таблицы без подстрок, с общей разницей 1..10%. Время, необходимое для регулярного поиска ключевых слов, должно быть почти таким же или идентичным для всех трех обсуждаемых типов индексов (CRC без подстрок, CRC с подстроками, ключевые слова). Время подстрочного поиска может значительно колебаться в зависимости от того, сколько фактических ключевых слов соответствует данной подстроке (т.е. сколько ключевых слов расширяется в поисковый запрос). Максимальное количество совпадающих ключевых слов ограничено директивой [expansion_limit](../../Server_settings/Searchd.md#expansion_limit).

В заключение, словари ключевых слов и CRC предлагают два различных компромиссных решения для подстрочного поиска. Вы можете решить либо пожертвовать временем индексации и размером индекса, чтобы добиться самых быстрых поисков в худшем случае (словарь CRC), либо минимально повлиять на время индексации, но пожертвовать временем поиска в худшем случае, когда префикс расширяется в большое количество ключевых слов (словарь ключевых слов).

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) dict = 'keywords'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) dict = 'keywords'");
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
Ограничение размера встроенных исключений, форм слов или стоп-слов. Необязательный, по умолчанию 16K.

Когда вы создаете таблицу, вышеупомянутые файлы могут быть либо сохранены внешне вместе с таблицей, либо встроены непосредственно в таблицу. Файлы размером менее `embedded_limit` хранятся в таблице. Для более крупных файлов хранятся только имена файлов. Это также упрощает перенос файлов таблицы на другую машину; вы можете обойтись, просто скопировав один файл.

С более мелкими файлами такое встраивание уменьшает количество внешних файлов, от которых зависит таблица, и помогает обслуживанию. Но в то же время нет смысла встраивать 100 МБ словарь форм слов в крошечную дельта-таблицу. Поэтому необходим порог размера, и `embedded_limit` является этим порогом.

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
Путь к файлу с глобальными (кластерными) IDF ключевых слов. Необязательный, по умолчанию пусто (использовать локальные IDF).

В кластере с несколькими таблицами частоты по ключевым словам, скорее всего, будут отличаться между разными таблицами. Это означает, что когда функция ранжирования использует значения, основанные на TF-IDF, такие как семья факторов BM25, результаты могут быть немного по-разному ранжированы в зависимости от того, на каком узле кластера они находятся.
Самый простой способ исправить эту проблему - создать и использовать глобальный словарь частоты, или файл глобального IDF. Эта директива позволяет указать местоположение этого файла. Рекомендуется (но не обязательно) использовать расширение .idf. Когда файл IDF указан для данной таблицы *и* [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) установлен в 1, движок будет использовать частоты ключевых слов и количество документов коллекции из файла global_idf, а не только из локальной таблицы. Таким образом, IDF и значения, которые зависят от них, останутся согласованными по всему кластеру.

Файлы IDF могут быть общими для нескольких таблиц. Только одна копия файла IDF будет загружена `searchd`, даже когда многие таблицы ссылаются на этот файл. Если содержимое файла IDF изменится, новое содержимое можно загрузить с помощью SIGHUP.

Вы можете создать файл .idf с использованием утилиты [indextool](../../Miscellaneous_tools.md#indextool), сначала выгрузив словари с помощью переключателя `--dumpdict dict.txt --stats`, затем конвертируя их в формат .idf с помощью `--buildidf`, а затем объединяя все файлы .idf по кластеру с помощью `--mergeidf`.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'");
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
Список слов без попаданий. Необязательно, допустимые значения: 'all' или имя файла списка.

По умолчанию полнотекстовый индекс Manticore хранит не только список соответствующих документов для каждого данного ключевого слова, но также и список его позиций в документах (известный как список попаданий). Списки попаданий позволяют выполнять поиск по фразам, по близости, строгому порядку и другим типам поиска, а также ранжировать по близости фраз. Однако списки попаданий для определенных частых ключевых слов (которые не могут быть исключены по каким-либо причинам, несмотря на частоту) могут стать огромными и, следовательно, замедлить обработку при запросах. Кроме того, в некоторых случаях нам может быть важно только булево соответствие ключевых слов, и нам никогда не понадобятся операторы поиска по позициям (такие как соответствие по фразам) или ранжирование по фразам.

`hitless_words` позволяет вам создавать индексы, которые либо вообще не имеют позиционной информации (списки попаданий), либо пропускают ее для конкретных ключевых слов.

Индекс без попаданий обычно занимает меньше места, чем соответствующий обычный полнотекстовый индекс (можно ожидать около 1.5x). Как индексирование, так и поиск должны быть быстрее, за счет отсутствия поддержки позиционных запросов и ранжирования.  

Если использовать в позиционных запросах (например, запросах по фразам), слова без попаданий извлекаются из них и используются в качестве операнда без позиции. Например, если "hello" и "world" являются словами без попаданий, а "simon" и "says" - нет, то фразовый запрос `"simon says hello world"` будет преобразован в `("simon says" & hello & world)`, соответствуя "hello" и "world" где угодно в документе и "simon says" как точной фразе.

Позиционный запрос, который содержит только слова без попаданий, приведет к пустому узлу фразы, поэтому весь запрос вернет пустой результат и предупреждение. Если весь словарь без попаданий (с использованием `all`), можно использовать только булевое соответствие на соответствующем индексе.



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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words = 'all'");
```ini
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
Включает вычисление и сохранение длин полей (как для каждого документа, так и средних значений для индекса) в полнотекстовый индекс. Опционально, по умолчанию 0 (не вычислять и не сохранять).
Когда `index_field_lengths` установлено в 1, Manticore будет:
* создавать соответствующий атрибут длины для каждого полнотекстового поля с тем же именем, но с суффиксом `__len`
* вычислять длину поля (подсчитываемую в ключевых словах) для каждого документа и сохранять в соответствующий атрибут
* вычислять средние значения для индекса. Атрибуты длины будут иметь специальный тип TOKENCOUNT, но их значения фактически являются обычными 32-битными целыми числами, к которым можно обычно обращаться.
Функции [BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) и [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) в выражении ранжировщика основаны на этих длинах и требуют, чтобы `index_field_lengths` был включен. Исторически Manticore использовал упрощенный, облегченный вариант BM25, который, в отличие от полной функции, **не** учитывал длину документа. Также поддерживается как полноценный вариант BM25, так и его расширение для работы с несколькими полями, называемое BM25F. Они требуют длину для каждого документа и для поля соответственно. Отсюда дополнительная директива.
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
##### Javascript:
<!-- request javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'');
```
<!-- intro -->
##### java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'");
```
<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'");
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
Фильтр токенов на этапе индексации для полнотекстового индексирования. Необязательно, по умолчанию пусто.
Директива index_token_filter указывает дополнительный фильтр токенов на этапе индексации для полнотекстового индексирования. Эта директива используется для создания настраиваемого токенизатора, который создает токены в соответствии с настраиваемыми правилами. Фильтр создается индексатором при индексировании исходных данных в обычную таблицу или RT таблицей при обработке операторов `INSERT` или `REPLACE`. Плагины определяются с использованием формата `имя библиотеки:имя плагина:опциональная строка настроек`. Например, `my_lib.so:custom_blend:chars=@#&`.
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
##### Javascript:
<!-- request javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'');
```
<!-- intro -->
##### java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'");
```
<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'");
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
Приращение позиции по пересортице (меньше чем [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)) ключевых слов. Необязательно, разрешенные значения 0 и 1, по умолчанию 1.
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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) overshort_step = '1'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) overshort_step = '1'");
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
phrase_boundary = ., ?, !, U+2026 # горизонтальное многоточие
```

<!-- example phrase_boundary -->
Список символов границ фраз. Необязательно, по умолчанию пусто.

Этот список управляет тем, какие символы будут рассматриваться как границы фраз, чтобы корректировать позиции слов и обеспечивать эмуляцию поиска на уровне фраз с помощью поискового запроса по близости. Синтаксис аналогичен [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), но сопоставления не допускаются, а символы границы не должны пересекаться с чем-либо еще.

На границе фразы дополнительное увеличение позиции слова (указанное с помощью [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)) будет добавлено к текущей позиции слова. Это позволит выполнять поиск на уровне фраз с помощью запросов по близости: слова в разных фразах будут гарантированно находиться более чем на расстоянии phrase_boundary_step друг от друга; поэтому поиск по близости на этом расстоянии будет эквивалентен поиску на уровне фраз.

Условие границы фразы будет поднято, если и только если такой символ будет за которым следует разделитель; это сделано для того, чтобы избежать сокращений, таких как S.T.A.L.K.E.R или URL, которые могли бы рассматриваться как несколько фраз.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'");
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
Увеличение позиции слова границы фразы. Необязательно, по умолчанию 0.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'");
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
# индексировать '13"' как '13дюймов'
regexp_filter = \b(\d+)\" => \1дюйм

# индексировать 'синий' или 'красный' как 'цвет'
regexp_filter = (blue|red) => color
```

<!-- example regexp_filter -->
Регулярные выражения (regexps), используемые для фильтрации полей и запросов. Эта директива необязательна, может иметь несколько значений, и по умолчанию представляет собой пустой список регулярных выражений. Движок регулярных выражений, используемый Manticore Search, - это RE2 от Google, который известен своей скоростью и безопасностью. Для получения подробной информации о синтаксисе, поддерживаемом RE2, вы можете посетить [руководство по синтаксису RE2](https://github.com/google/re2/wiki/Syntax).

В определенных приложениях, таких как поиск продуктов, может быть много способов ссылаться на продукт, модель или свойство. Например, `iPhone 3gs` и `iPhone 3 gs` (или даже `iPhone3 gs`) очень вероятно относятся к одному и тому же продукту. Другим примером могут быть разные способы обозначения размера экрана ноутбука, такие как `13-дюймовый`, `13 дюймов`, `13"` или `13дюйм`.

Regexps предоставляют механизм для определения правил, предназначенных для обработки таких случаев. В первом примере вы, возможно, могли бы использовать файл словообразований, чтобы обработать несколько моделей iPhone, но во втором примере лучше указать правила, которые будут нормализовать "13-дюймовый" и "13дюйм" в нечто идентичное.

Регулярные выражения, указанные в `regexp_filter`, применяются в порядке их перечисления, на самом раннем этапе, до любого другого процесса (включая [исключения](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions)), даже до токенизации. То есть регулярные выражения применяются к сырым исходным полям при индексировании и к сырому тексту поискового запроса при поиске.

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'");
```

<!-- request CONFIG -->

```ini
table products {
  # индексировать '13"' как '13дюймов'
  regexp_filter = \b(\d+)\" => \1дюйм

  # индексировать 'синий' или 'красный' как 'цвет'
  regexp_filter = (blue|red) => color

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->



























































