# Расширенная токенизация HTML

## Удаление HTML-тегов

### html_strip

<!-- example html_strip -->

```ini
html_strip = {0|1}
```

Эта опция определяет, следует ли удалять HTML-разметку из входных данных полного текста. Значение по умолчанию - 0, что отключает удаление. Чтобы включить удаление, установите значение на 1.

HTML-теги и сущности рассматриваются как разметка и будут обработаны. 

HTML-теги удаляются, в то время как содержимое между ними (например, все между `<p>` и `</p>`) остается нетронутым. Вы можете выбрать, нужно ли сохранять и индексировать атрибуты тегов (например, атрибут HREF в теге A или ALT в теге IMG). Некоторые известные встроенные теги, такие как A, B, I, S, U, BASEFONT, BIG, EM, FONT, IMG, LABEL, SMALL, SPAN, STRIKE, STRONG, SUB, SUP и TT, полностью удаляются. Все остальные теги обрабатываются как блочные и заменяются пробелами. Например, текст `te<b>st</b>` будет индексироваться как одно ключевое слово 'test', в то время как `te<p>st</p>` будет индексироваться как два ключевых слова 'te' и 'st'.

HTML-сущности декодируются и заменяются соответствующими символами UTF-8. Удалитель поддерживает как числовые формы (например, `&#239;`), так и текстовые формы (например, `&oacute;` или `&nbsp;`) сущностей и поддерживает все сущности, указанные в стандарте HTML4.

Удалитель предназначен для работы с правильно оформленным HTML и XHTML, но может выдавать неожиданные результаты при неправильном входе (например, HTML с лишними `<'s` или незакрытыми `>'s`).

Пожалуйста, обратите внимание, что удаляются только сами теги, а также HTML-комментарии. Чтобы удалить содержимое тегов, включая встроенные скрипты, смотрите опцию [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements). Нет ограничений на имена тегов, что означает, что все, что выглядит как действительный старт, конец или комментарий тега, будет удалено.


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

Опция html_index_attrs позволяет вам указать, какие атрибты HTML-разметки следует индексировать, даже если другая HTML-разметка удалена. Значение по умолчанию - пустое, что означает, что атрибты индексироваться не будут. 
Формат опции - это перечисление индексируемых атрибтов по тегам, как показано в приведенном выше примере. Содержимое указанных атрибутов будет сохранено и проиндексировано, обеспечивая способ извлечения дополнительной информации из ваших данных полного текста.


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
Список HTML-элементов, содержимое которых, вместе с самими элементами, будет удалено. Необязательно, по умолчанию является пустой строкой (не удалять содержимое никаких элементов).

Эта опция позволяет вам удалить содержимое элементов, то есть все, что находится между открывающими и закрывающими тегами. Это полезно для удаления встроенных скриптов, CSS и т.д. Краткая форма тега для пустых элементов (например, <br/>) поддерживается корректно, и текст, следующий за таким тегом, не будет удален.

Значение - это список имен элементов (теней), содержимое которых следует удалить, разделенных запятыми. Имена тегов нечувствительны к регистру.


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

Управляет обнаружением и индексированием границ предложений и абзацев. Необязательно, по умолчанию 0 (без обнаружения или индексирования).

Эта директива позволяет обнаруживать и индексировать границы предложений и абзацев, что делает возможным работу операторов [SENTENCE](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators)  и [PARAGRAPH](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators). Обнаружение границ предложений основано на анализе простого текста и требует только установки `index_sp = 1`, чтобы включить его. Однако для обнаружения абзацев требуется HTML-разметка и происходит во время [процесса удаления HTML](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip). Таким образом, для индексирования границ абзацев и директива index_sp, и директива html_strip должны быть установлены в 1.

Для определения границ предложений используются следующие правила:

* Вопросительные знаки (?) и восклицательные знаки (!) всегда указывают на конец предложения. 
* Завершающие точки (.) указывают на конец предложения, за исключением следующих случаев:
    * Когда за ними следует буква. Это считается частью аббревиатуры (например, "S.T.A.L.K.E.R." или "Goldman Sachs S.p.A.").
    * Когда за ними следует запятая. Это считается аббревиатурой, за которой следует запятая (например, "Telecom Italia S.p.A., основанная в 1994 году").
    * Когда за ними следует пробел и строчная буква. Это считается аббревиатурой внутри предложения (например, "News Corp. объявила в феврале").
    * Когда предшествует пробел и заглавная буква, и за ней следует пробел. Это считается средним инициалом (например, "John D. Doe").
        
Границы абзацев определяются при каждом блочном HTML-теге, включая: ADDRESS, BLOCKQUOTE, CAPTION, CENTER, DD, DIV, DL, DT, H1, H2, H3, H4, H5, LI, MENU, OL, P, PRE, TABLE, TBODY, TD, TFOOT, TH, THEAD, TR и UL.

Оба, предложения и абзацы, увеличивают счетчик позиций ключевых слов на 1.


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = '1'");
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

Список зон HTML/XML в поле для индексации. По умолчанию используется пустая строка (зоны не будут индексированы).

"Зона" определяется как всё между открывающим и соответствующим закрывающим тегом, и все диапазоны, имеющие одно и то же имя тега, называются "зоной." Например, всё между `<H1>` и `</H1>` в поле документа относится к зоне H1.

Директива `index_zones` позволяет индексировать зоны, но HTML [stripper](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) также должен быть включен (установив `html_strip = 1`). Значение `index_zones` должно быть списком имен тегов и подстановочных знаков, разделённых запятыми (оканчиваясь звездочкой), которые будут индексированы как зоны.

Зоны могут быть вложенными и перекрывающимися, при условии, что каждый открывающий тег имеет соответствующий тег. Зоны также могут использоваться для сопоставления с оператором ZONE, как описано в [extended_query_syntax](../../Searching/Full_text_matching/Operators.md#ZONE-limit-operator).


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = \'h, th, title\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'");
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

