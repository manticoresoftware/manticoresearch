# 高级 HTML 令牌化

## 去除 HTML 标签

### html_strip

<!-- example html_strip -->

```ini
html_strip = {0|1}
```

此选项确定是否应从传入的全文数据中去除 HTML 标记。默认值为 0，这会禁用去除。如果要启用去除，请将值设置为 1。

HTML 标签和实体被视为标记并将被处理。 

HTML 标签被去除，而它们之间的内容（例如 `<p>` 和 `</p>` 之间的所有内容）将保持不变。您可以选择保留和索引标签属性（例如 A 标签中的 HREF 属性或 IMG 标签中的 ALT 属性）。一些知名的内联标签，如 A、B、I、S、U、BASEFONT、BIG、EM、FONT、IMG、LABEL、SMALL、SPAN、STRIKE、STRONG、SUB、SUP 和 TT，都会被完全去除。所有其他标签被视为区块级，并用空格替换。例如，文本 `te<b>st</b>` 将被索引为一个单一的关键字 'test'，而 `te<p>st</p>` 将被索引为两个关键字 'te' 和 'st'。

HTML 实体被解码并替换为其对应的 UTF-8 字符。该处理器支持数字形式（例如 `&#239;`）和文本形式（例如 `&oacute;` 或 `&nbsp;`）的实体，并支持 HTML4 标准定义的所有实体。

该处理器旨在处理格式正确的 HTML 和 XHTML，但可能会对格式不正确的输入（如包含多余的 `<'s` 或未闭合的 `>'s` 的 HTML）产生意外结果。

请注意，只有标签本身以及 HTML 注释会被去除。要去除标签内容，包括嵌入的脚本，请参见 [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements) 选项。对标签名没有限制，这意味着任何看起来像有效标签开始、结束或注释的内容都将被去除。


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

html_index_attrs 选项允许您指定应被索引的 HTML 标记属性，即使其他 HTML 标记被去除。默认值为空，意味着不会索引任何属性。 
该选项的格式是每个标签的可索引属性的枚举，如上面的示例所示。指定属性的内容将被保留和索引，从而提供一种从您的全文数据中提取额外信息的方法。


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
A list of HTML elements whose contents, along with the elements themselves, will be stripped. Optional, the default is an empty string (do not strip contents of any elements).

此选项允许您删除元素的内容，意味着所有内容在开闭标签之间。这对于删除嵌入的脚本、CSS等非常有用。对空元素的短标签形式（例如 <br/>）得到正确支持，紧随其后这样的标签的文本将不会被删除。

该值是一个以逗号分隔的元素（标签）名称列表，这些来自它们的内容将被移除。标签名称不区分大小写。


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

## Extracting important parts from HTML

### index_sp

<!-- example index_sp -->

```ini
index_sp = {0|1}
```

Controls detection and indexing of sentence and paragraph boundaries. Optional, default is 0 (no detection or indexing).

该指令使得句子和段落边界的检测和索引成为可能，这使得[SENTENCE](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators)和[PARAGRAPH](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators)操作可以工作。句子边界检测是基于纯文本分析，只需设置`index_sp = 1`来启用它。然而，段落检测依赖于HTML标记，并在[HTML剥离过程](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip)中发生。因此，要对段落边界进行索引，必须将index_sp指令和html_strip指令都设置为1。

确定句子边界使用以下规则：

* 问号(?)和感叹号(!)总是表示句子边界。
* 结尾的点(.)表示句子边界，除非在以下情况下：
    * 当后面跟着一个字母时。这被认为是缩写的一部分（例如"S.T.A.L.K.E.R."或"Goldman Sachs S.p.A."）。
    * 当后面跟着一个逗号时。这被认为是缩写后跟一个逗号（例如"Telecom Italia S.p.A., founded in 1994"）。
    * 当后面跟着一个空格和一个小写字母时。这被认为是句子中的缩写（例如"News Corp. announced in February"）。
    * 当前面是一个空格和一个大写字母，并且后面是一个空格。这被认为是中间名（例如"John D. Doe"）。
        
段落边界在每个块级HTML标签处被检测，包括：ADDRESS、BLOCKQUOTE、CAPTION、CENTER、DD、DIV、DL、DT、H1、H2、H3、H4、H5、LI、MENU、OL、P、PRE、TABLE、TBODY、TD、TFOOT、TH、THEAD、TR和UL。

句子和段落都将关键字位置计数器增加1。


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

在要索引的字段内的 HTML/XML 区域列表。默认值是空字符串（没有区域将被索引）。

“区域”被定义为开标签和匹配闭标签之间的所有内容，所有共享相同标签名称的跨度被称为“区域”。例如，文档字段中 `<H1>` 和 `</H1>` 之间的所有内容属于 H1 区域。

`index_zones` 指令启用区域索引，但 HTML [剥离器](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) 也必须启用（通过设置 `html_strip = 1`）。`index_zones` 的值应为要作为区域索引的标签名称和通配符（以星号结尾）的逗号分隔列表。

区域可以嵌套和重叠，只要每个开标签都有一个匹配的标签。区域还可以用于与 ZONE 运算符匹配，如 [extended_query_syntax](../../Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 中所述。


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

