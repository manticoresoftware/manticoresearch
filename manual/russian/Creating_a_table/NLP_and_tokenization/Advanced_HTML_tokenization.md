# Расширенная токенизация HTML

## Удаление HTML тегов

### html_strip

<!-- example html_strip -->

```ini
html_strip = {0|1}
```

Этот параметр определяет, следует ли удалять HTML-разметку из входных полнотекстовых данных. Значение по умолчанию — 0, что отключает удаление. Чтобы включить удаление, установите значение в 1.

HTML теги и сущности считаются разметкой и будут обработаны.

HTML теги удаляются, а содержимое между ними (например, всё между `<p>` и `</p>`) остаётся нетронутым. Вы можете выбрать сохранение и индексирование атрибутов тегов (например, атрибут HREF в теге A или ALT в теге IMG). Некоторые известные встроенные теги, такие как A, B, I, S, U, BASEFONT, BIG, EM, FONT, IMG, LABEL, SMALL, SPAN, STRIKE, STRONG, SUB, SUP и TT, полностью удаляются. Все остальные теги рассматриваются как блочные и заменяются пробелом. Например, текст `te<b>st</b>` будет проиндексирован как одно ключевое слово 'test', а `te<p>st</p>` будет проиндексирован как два ключевых слова 'te' и 'st'.

HTML-сущности декодируются и заменяются соответствующими символами UTF-8. Удаление поддерживает как числовые формы (например, `&#239;`), так и текстовые формы (например, `&oacute;` или `&nbsp;`) сущностей и поддерживает все сущности, определённые стандартом HTML4.

Удаление предназначено для работы с корректно сформированным HTML и XHTML, но может давать неожиданные результаты при некорректном вводе (например, HTML с посторонними `<'s` или незакрытыми `>'s`).

Обратите внимание, что удаляются только сами теги, а также HTML-комментарии. Чтобы удалить содержимое тегов, включая встроенные скрипты, смотрите опцию [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements). Нет ограничений на имена тегов, всё, что выглядит как допустимое начало тега, конец тега или комментарий, будет удалено.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) html_strip = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_strip = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_strip = '1'");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) html_strip = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  html_strip = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### html_index_attrs

<!-- example html_index_attrs -->

```ini
html_index_attrs = img=alt,title; a=title;
```

Опция html_index_attrs позволяет указать, какие атрибуты HTML-разметки должны индексироваться, даже если остальная HTML-разметка удаляется. Значение по умолчанию — пустое, что означает отсутствие индексируемых атрибутов.
Формат опции — перечисление индексируемых атрибутов для каждого тега, как показано в примере выше. Содержимое указанных атрибутов будет сохранено и проиндексировано, что позволяет извлекать дополнительную информацию из полнотекстовых данных.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'html_index_attrs' => 'img=alt,title; a=title;',
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = '1'");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  html_index_attrs = img=alt,title; a=title;
  html_strip = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### html_remove_elements

<!-- example html_remove_elements -->

```ini
html_remove_elements = element1[, element2, ...]
```

Список HTML-элементов, содержимое которых вместе с самими элементами будет удалено. Опционально, значение по умолчанию — пустая строка (не удалять содержимое ни одного элемента).

Эта опция позволяет удалить содержимое элементов, то есть всё между открывающим и закрывающим тегами. Это полезно для удаления встроенных скриптов, CSS и т. п. Короткая форма записи пустых элементов (например, <br/>) поддерживается корректно, и текст после такого тега не удаляется.

Значение — это список через запятую имён элементов (тегов), содержимое которых должно быть удалено. Имена тегов не чувствительны к регистру.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'html_remove_elements' => 'style, script',
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = '1'");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  html_remove_elements = style, script
  html_strip = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## Извлечение важных частей из HTML

### index_sp

<!-- example index_sp -->

```ini
index_sp = {0|1}
```

Управляет обнаружением и индексированием границ предложений и абзацев. Опционально, значение по умолчанию — 0 (обнаружение и индексирование отключено).

Эта директива включает обнаружение и индексирование границ предложений и абзацев, что позволяет работать операторам [SENTENCE](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) и [PARAGRAPH](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators). Обнаружение границ предложений основано на анализе обычного текста и требует только установки `index_sp = 1` для включения. Обнаружение абзацев, однако, зависит от HTML-разметки и происходит во время процесса [удаления HTML](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip). Таким образом, для индексирования границ абзацев необходимо установить директивы index_sp и html_strip в 1.

Для определения границ предложений используются следующие правила:

* Вопросительные (?) и восклицательные (!) знаки всегда указывают на границу предложения.
* Точки в конце предложений (.) указывают на границу предложения, за исключением следующих случаев:
    * Когда за ним следует буква. Это считается частью аббревиатуры (например, "S.T.A.L.K.E.R." или "Goldman Sachs S.p.A.").
    * Когда за ним следует запятая. Это считается аббревиатурой, за которой следует запятая (например, "Telecom Italia S.p.A., основанная в 1994 году").
    * Когда за ним следует пробел и строчная буква. Это считается аббревиатурой в пределах предложения (например, "News Corp. объявила в феврале").
    * Когда перед ним стоит пробел и заглавная буква, а после — пробел. Это считается инициалом (например, "John D. Doe").

Границы абзацев определяются на каждом блочном теге HTML, включая: ADDRESS, BLOCKQUOTE, CAPTION, CENTER, DD, DIV, DL, DT, H1, H2, H3, H4, H5, LI, MENU, OL, P, PRE, TABLE, TBODY, TD, TFOOT, TH, THEAD, TR и UL.

И предложения, и абзацы увеличивают счётчик позиции ключевого слова на 1.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_sp' => '1',
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = '1'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  index_sp = 1
  html_strip = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->


### index_zones

<!-- example index_zones -->

```ini
index_zones = h*, th, title
```

Список HTML/XML зон в поле, которые должны индексироваться. По умолчанию это пустая строка (зоны не индексируются).

«Зона» определяется как всё, что находится между открывающим и соответствующим закрывающим тегом, и все спаны с одинаковым именем тега относятся к одной «зоне». Например, всё, что находится между `<H1>` и `</H1>` в поле документа, принадлежит зоне H1.

Директива `index_zones` включает индексацию зон, но должен быть также включён [HTML-стриппер](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) (установка `html_strip = 1`). Значение `index_zones` должно быть списком имён тегов и подстановочных символов (оканчивающихся на звёздочку), разделённых запятыми, которые будут индексироваться как зоны.

Зоны могут вкладываться и пересекаться, при условии, что у каждого открывающего тега есть соответствующий закрывающий. Зоны также могут использоваться для поиска с помощью оператора ZONE, как описано в [расширенном синтаксисе запросов](../../Searching/Full_text_matching/Operators.md#ZONE-limit-operator).


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_zones' => 'h*,th,title',
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = \'h, th, title\' html_strip = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = \'h, th, title\' html_strip = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = \'h, th, title\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  index_zones = h*, th, title
  html_strip = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->

