# 高级 HTML 分词

## 去除 HTML 标签

### html_strip

<!-- example html_strip -->

```ini
html_strip = {0|1}
```

此选项决定是否应从传入的全文数据中去除 HTML 标记。默认值为 0，表示禁用去除。要启用去除，请将值设置为 1。

HTML 标签和实体被视为标记并将被处理。

HTML 标签会被移除，而它们之间的内容（例如 `<p>` 和 `</p>` 之间的所有内容）则保持不变。您可以选择保留并索引标签属性（例如 A 标签中的 HREF 属性或 IMG 标签中的 ALT 属性）。一些知名的内联标签，如 A、B、I、S、U、BASEFONT、BIG、EM、FONT、IMG、LABEL、SMALL、SPAN、STRIKE、STRONG、SUB、SUP 和 TT，会被完全移除。所有其他标签被视为块级标签，并被替换为空白。例如，文本 `te<b>st</b>` 会被索引为单个关键词 'test'，而 `te<p>st</p>` 会被索引为两个关键词 'te' 和 'st'。

HTML 实体会被解码并替换为对应的 UTF-8 字符。去除器支持数字形式的实体（例如 `&#239;`）和文本形式的实体（例如 `&oacute;` 或 `&nbsp;`），并支持 HTML4 标准指定的所有实体。

该去除器设计用于处理格式正确的 HTML 和 XHTML，但在处理格式错误的输入（如带有多余的 `<` 或未闭合的 `>` 的 HTML）时可能产生意外结果。

请注意，只有标签本身以及 HTML 注释会被去除。要去除标签内容，包括嵌入的脚本，请参见 [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements) 选项。标签名称没有限制，意味着所有看起来像有效标签开始、结束或注释的内容都会被去除。


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

html_index_attrs 选项允许您指定即使其他 HTML 标记被去除，也应被索引的 HTML 标记属性。默认值为空，表示不索引任何属性。
该选项的格式是按标签列举可索引属性，如上例所示。指定属性的内容将被保留并索引，提供了一种从全文数据中提取额外信息的方法。


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

一个 HTML 元素列表，其内容及元素本身将被去除。可选，默认值为空字符串（不去除任何元素内容）。

此选项允许您去除元素的内容，即开闭标签之间的所有内容。它对于去除嵌入的脚本、CSS 等非常有用。空元素的短标签形式（例如 <br/>）被正确支持，且该标签之后的文本不会被去除。

该值是以逗号分隔的元素（标签）名称列表，指定应去除其内容。标签名称不区分大小写。


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

## 从 HTML 中提取重要部分

### index_sp

<!-- example index_sp -->

```ini
index_sp = {0|1}
```

控制句子和段落边界的检测和索引。可选，默认值为 0（不检测也不索引）。

此指令启用句子和段落边界的检测和索引，使得 [SENTENCE](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) 和 [PARAGRAPH](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) 操作符能够工作。句子边界检测基于纯文本分析，只需设置 `index_sp = 1` 即可启用。段落检测则依赖于 HTML 标记，并在 [HTML 去除过程](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) 中进行。因此，要索引段落边界，必须同时将 index_sp 指令和 html_strip 指令设置为 1。

以下规则用于确定句子边界：

* 问号 (?) 和感叹号 (!) 总是表示句子边界。
* 句末点号 (.) 表示句子边界，除以下情况外：
    * 当后面跟着一个字母时。这被视为缩写的一部分（例如 "S.T.A.L.K.E.R." 或 "Goldman Sachs S.p.A."）。
    * 当后面跟着一个逗号时。这被视为缩写后跟逗号（例如 "Telecom Italia S.p.A., founded in 1994"）。
    * 当后面跟着一个空格和一个小写字母时。这被视为句子中的缩写（例如 "News Corp. announced in February"）。
    * 当前面跟着一个空格和一个大写字母，且后面跟着一个空格时。这被视为中间名的首字母（例如 "John D. Doe"）。

段落边界在每个块级 HTML 标签处检测，包括：ADDRESS、BLOCKQUOTE、CAPTION、CENTER、DD、DIV、DL、DT、H1、H2、H3、H4、H5、LI、MENU、OL、P、PRE、TABLE、TBODY、TD、TFOOT、TH、THEAD、TR 和 UL。

句子和段落都会使关键字位置计数器增加 1。


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

字段内要索引的 HTML/XML 区域列表。默认值为空字符串（不索引任何区域）。

“区域”定义为开标签和匹配的闭标签之间的所有内容，所有共享相同标签名称的跨度都称为一个“区域”。例如，文档字段中 `<H1>` 和 `</H1>` 之间的所有内容属于 H1 区域。

`index_zones` 指令启用区域索引，但必须同时启用 HTML [剥离器](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip)（通过设置 `html_strip = 1`）。`index_zones` 的值应为以逗号分隔的标签名称和通配符（以星号结尾）的列表，用于作为区域索引。

区域可以嵌套和重叠，只要每个开标签都有匹配的闭标签。区域也可以用于使用 ZONE 操作符进行匹配，如 [extended_query_syntax](../../Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 中所述。


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

