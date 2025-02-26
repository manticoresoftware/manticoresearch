# 高级 HTML 标记化

## 删除 HTML 标签

### html_strip

<!-- example html_strip -->

```ini
html_strip = {0|1}
```

此选项用于确定是否从传入的全文数据中删除 HTML 标记。默认值为 0，表示不删除。如果要启用删除功能，请将该值设置为 1。

HTML 标签和实体会被视为标记并进行处理。

HTML 标签会被删除，而标签之间的内容（例如 `<p>` 和 `</p>` 之间的内容）将被保留。您可以选择保留并索引标签属性（例如 A 标签中的 HREF 属性或 IMG 标签中的 ALT 属性）。一些常见的内联标签，如 A、B、I、S、U、BASEFONT、BIG、EM、FONT、IMG、LABEL、SMALL、SPAN、STRIKE、STRONG、SUB、SUP 和 TT，会被完全删除。所有其他标签被视为块级标签，并替换为空格。例如，文本 `te<b>st</b>` 将被索引为单个关键字“test”，而 `te<p>st</p>` 将被索引为两个关键字“te”和“st”。

HTML 实体会被解码并替换为其对应的 UTF-8 字符。剥离器支持数字形式（例如 `ï`）和文本形式（例如 `ó` 或 ` `）的实体，并支持 HTML4 标准中规定的所有实体。

该功能设计用于处理格式正确的 HTML 和 XHTML，但在处理格式不正确的输入时（例如带有多余的 `<` 或未闭合的 `>` 的 HTML），可能会产生意外结果。

请注意，只有标签本身以及 HTML 注释会被删除。要删除标签内容，包括嵌入的脚本，请参阅 [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements) 选项。标签名没有任何限制，这意味着所有看起来像有效标签开始、结束或注释的内容都会被删除。

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

`html_index_attrs` 选项允许您指定即使其他 HTML 标记被删除，哪些 HTML 标记属性仍应被索引。默认值为空，表示不会索引任何属性。

该选项的格式是按标签列举可索引的属性，如上例所示。指定属性的内容将被保留并索引，从而为您的全文数据提取附加信息。


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

`html_remove_elements` 选项用于指定要删除内容及其元素本身的 HTML 元素列表。该选项是可选的，默认值为空字符串（表示不删除任何元素的内容）。

此选项允许您删除元素的内容，即删除打开和关闭标签之间的所有内容。它在删除嵌入的脚本、CSS 等方面非常有用。对于空元素的短标签形式（如 `<br/>`）也有正确支持，并且该标签之后的文本不会被删除。

值是一个以逗号分隔的元素（标签）名称列表，应该删除这些标签的内容。标签名称不区分大小写。

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

## 从 HTML 中提取重要部分

### index_sp

<!-- example index_sp -->

```ini
index_sp = {0|1}
```

`index_sp` 选项控制句子和段落边界的检测与索引。该选项是可选的，默认值为 0（不进行检测或索引）。

此指令启用句子和段落边界的检测与索引，使 [SENTENCE](../../Searching/Full_text_matching/Operators.md#SENTENCE-和-PARAGRAPH-操作符) 和 [PARAGRAPH](../../Searching/Full_text_matching/Operators.md#SENTENCE-和-PARAGRAPH-操作符) 操作符可以正常工作。句子边界的检测基于纯文本分析，只需设置 `index_sp = 1` 即可启用。段落检测则依赖于 HTML 标记，并在 [HTML 剥离过程](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) 中进行。因此，要索引段落边界，必须将 `index_sp` 和 `html_strip` 两个指令都设置为 1。

以下规则用于确定句子边界：

- 问号 (?) 和感叹号 (!) 总是表示句子边界。
- 尾随的句号 (.) 表示句子边界，但在以下情况下除外：
  - 当句号后跟字母时，视为缩写的一部分（例如 "S.T.A.L.K.E.R." 或 "Goldman Sachs S.p.A."）。
  - 当句号后跟逗号时，视为缩写后紧跟逗号（例如 "Telecom Italia S.p.A., founded in 1994"）。
  - 当句号后跟空格和小写字母时，视为句中的缩写（例如 "News Corp. announced in February"）。
  - 当句号前有空格且后跟大写字母并且后跟空格时，视为名字的中间缩写（例如 "John D. Doe"）。

段落边界则在每个块级 HTML 标签处检测，包括：ADDRESS、BLOCKQUOTE、CAPTION、CENTER、DD、DIV、DL、DT、H1、H2、H3、H4、H5、LI、MENU、OL、P、PRE、TABLE、TBODY、TD、TFOOT、TH、THEAD、TR 和 UL。

句子和段落都会将关键词位置计数器递增 1。

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

`index_zones` 用于指定需要索引的 HTML/XML 区域列表。默认值为空字符串（即不索引任何区域）。

“区域”指的是从一个标签的开始到对应关闭标签之间的内容，所有共享同一标签名称的部分都被称为“区域”。例如，在文档字段中的 `<H1>` 和 `</H1>` 之间的所有内容都属于 H1 区域。

启用区域索引时，还需要同时启用 HTML [剥离器](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip)（通过设置 `html_strip = 1`）。`index_zones` 的值应为以逗号分隔的标签名称和通配符（以星号结尾），这些标签将作为区域进行索引。

区域可以嵌套或重叠，只要每个起始标签都有匹配的关闭标签即可。区域也可用于 `ZONE` 操作符进行匹配，详见 [扩展查询语法](../../Searching/Full_text_matching/Operators.md#ZONE-限制操作符)。


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