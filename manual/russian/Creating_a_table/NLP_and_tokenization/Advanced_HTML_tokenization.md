# Расширенная токенизация HTML

## Удаление HTML-тегов

### html_strip

<!-- example html_strip -->

```ini
html_strip = {0|1}
```

Этот параметр определяет, следует ли удалять HTML-разметку из входных полнотекстовых данных. Значение по умолчанию — 0, что отключает удаление. Чтобы включить удаление, установите значение в 1.

HTML-теги и сущности считаются разметкой и будут обработаны.

HTML-теги удаляются, а содержимое между ними (например, всё между `<p>` и `</p>`) остаётся нетронутым. Вы можете выбрать сохранение и индексирование атрибутов тегов (например, атрибут HREF в теге A или ALT в теге IMG). Некоторые известные встроенные теги, такие как A, B, I, S, U, BASEFONT, BIG, EM, FONT, IMG, LABEL, SMALL, SPAN, STRIKE, STRONG, SUB, SUP и TT, полностью удаляются. Все остальные теги считаются блочными и заменяются пробелом. Например, текст `te<b>st</b>` будет индексироваться как одно ключевое слово 'test', а `te<p>st</p>` — как два ключевых слова 'te' и 'st'.

HTML-сущности декодируются и заменяются соответствующими символами UTF-8. Удалитель поддерживает как числовые формы (например, `&#239;`), так и текстовые формы (например, `&oacute;` или `&nbsp;`) сущностей, а также все сущности, указанные в стандарте HTML4.

Удалитель предназначен для работы с корректно сформированным HTML и XHTML, но может давать неожиданные результаты при некорректном вводе (например, HTML с лишними символами `<` или незакрытыми `>`).

Обратите внимание, что удаляются только сами теги и HTML-комментарии. Чтобы удалить содержимое тегов, включая встроенные скрипты, смотрите опцию [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements). Нет ограничений на имена тегов, то есть всё, что выглядит как корректное начало, конец или комментарий тега, будет удалено.


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

Опция html_index_attrs позволяет указать, какие атрибуты HTML-разметки должны индексироваться, даже если остальная HTML-разметка удаляется. Значение по умолчанию — пустая строка, что означает, что атрибуты не индексируются.
Формат опции — перечисление индексируемых атрибутов для каждого тега, как показано в примере выше. Содержимое указанных атрибутов сохраняется и индексируется, что позволяет извлекать дополнительную информацию из полнотекстовых данных.


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

Список HTML-элементов, содержимое которых вместе с самими элементами будет удалено. Необязательно, значение по умолчанию — пустая строка (не удалять содержимое ни одного элемента).

Эта опция позволяет удалять содержимое элементов, то есть всё между открывающим и закрывающим тегами. Это полезно для удаления встроенных скриптов, CSS и т. п. Короткая форма тега для пустых элементов (например, <br/>) поддерживается корректно, и текст после такого тега удалён не будет.

Значение — список имён элементов (тегов), разделённых запятыми, содержимое которых следует удалить. Имена тегов не чувствительны к регистру.


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

Управляет обнаружением и индексированием границ предложений и абзацев. Необязательно, значение по умолчанию — 0 (обнаружение и индексирование отключены).

Эта директива включает обнаружение и индексирование границ предложений и абзацев, что позволяет работать операторам [SENTENCE](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) и [PARAGRAPH](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators). Обнаружение границ предложений основано на анализе простого текста и требует только установки `index_sp = 1` для включения. Обнаружение абзацев, однако, опирается на HTML-разметку и происходит во время [процесса удаления HTML](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip). Поэтому для индексирования границ абзацев обе директивы — index_sp и html_strip — должны быть установлены в 1.

Для определения границ предложений используются следующие правила:

* Вопросительные знаки (?) и восклицательные знаки (!) всегда указывают на границу предложения.
* Заканчивающиеся точки (.) указывают на границу предложения, за исключением следующих случаев:
    * Когда за ним следует буква. Это считается частью аббревиатуры (например, "S.T.A.L.K.E.R." или "Goldman Sachs S.p.A.").
    * Когда за ним следует запятая. Это считается аббревиатурой, за которой следует запятая (например, "Telecom Italia S.p.A., основанная в 1994 году").
    * Когда за ним следует пробел и строчная буква. Это считается аббревиатурой внутри предложения (например, "News Corp. объявила в феврале").
    * Когда перед ним стоит пробел и заглавная буква, а после него пробел. Это считается инициалом среднего имени (например, "John D. Doe").

Границы абзацев определяются на каждом блочном HTML-теге, включая: ADDRESS, BLOCKQUOTE, CAPTION, CENTER, DD, DIV, DL, DT, H1, H2, H3, H4, H5, LI, MENU, OL, P, PRE, TABLE, TBODY, TD, TFOOT, TH, THEAD, TR и UL.

И предложения, и абзацы увеличивают счетчик позиции ключевого слова на 1.


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

Список HTML/XML зон внутри поля для индексирования. По умолчанию это пустая строка (зоны не будут индексироваться).

«Зона» определяется как все, что находится между открывающим и соответствующим закрывающим тегом, и все спаны с одинаковым именем тега называются «зоной». Например, все, что находится между `<H1>` и `</H1>` в поле документа, принадлежит зоне H1.

Директива `index_zones` включает индексирование зон, но HTML [stripper](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) также должен быть включен (установкой `html_strip = 1`). Значение `index_zones` должно быть списком через запятую имен тегов и подстановочных знаков (заканчивающихся звездочкой), которые будут индексироваться как зоны.

Зоны могут быть вложенными и перекрываться, при условии, что каждый открывающий тег имеет соответствующий закрывающий тег. Зоны также могут использоваться для сопоставления с оператором ZONE, как описано в [extended_query_syntax](../../Searching/Full_text_matching/Operators.md#ZONE-limit-operator).


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

