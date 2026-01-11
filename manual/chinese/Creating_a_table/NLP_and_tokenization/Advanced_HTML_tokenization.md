# 高级HTML分词

## 去除HTML标签

### html_strip

<!-- example html_strip -->

```ini
html_strip = {0|1}
```

此选项确定是否应从传入的全文数据中去除HTML标记。默认值为0，表示禁用去除。要启用去除，请将值设置为1。

HTML标签和实体被视为标记，并将被处理。

HTML标签被移除，而它们之间的内容（例如，`<p>`和`</p>`之间的内容）保持不变。可以选择保留并索引标签属性（例如，A标签中的HREF属性或IMG标签中的ALT属性）。一些常见的内联标签，如A、B、I、S、U、BASEFONT、BIG、EM、FONT、IMG、LABEL、SMALL、SPAN、STRIKE、STRONG、SUB、SUP和TT，将完全移除。所有其他标签被视为块级标签，并用空格替换。例如，文本`te<b>st</b>`将被索引为单个关键词'test'，而`te<p>st</p>`将被索引为两个关键词'te'和'st'。

HTML实体被解码并替换为其相应的UTF-8字符。去除器支持实体的数字形式（例如`&#239;`）和文本形式（例如`&oacute;`或`&nbsp;`），并支持HTML4标准中指定的所有实体。

去除器旨在与正确形成的HTML和XHTML一起使用，但在处理不规范的输入（例如带有游离的`<'s`或未关闭的`>'s`的HTML）时可能会产生意外结果。

请注意，仅移除标签本身以及HTML注释。要移除标签的内容，包括嵌入的脚本，请参阅[html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements)选项。标签名称没有限制，这意味着任何看起来是有效标签开始、结束或注释的内容都将被移除。


<!-- intro -->
##### SQL：

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = \'1\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = \'1\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = \'1\'');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_strip = '1'");
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_strip = '1'");
```

<!-- intro -->
##### Rust：

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

html_index_attrs选项允许您指定即使其他HTML标记被去除，哪些HTML标记属性也应被索引。默认值为空，表示不会索引任何属性。
该选项的格式是每个标签的可索引属性的枚举，如上例所示。指定属性的内容将被保留并索引，从而提供从全文数据中提取附加信息的方法。


<!-- intro -->
##### SQL：

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = \'1\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = \'1\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = \'1\'');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = '1'");
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = '1'");
```

<!-- intro -->
##### Rust：

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

一个HTML元素列表，其内容及其自身将被移除。可选，默认值为空字符串（不移除任何元素的内容）。

此选项允许您移除元素的内容，即移除它们之间的所有内容。这对于移除嵌入的脚本、CSS等非常有用。空元素的简短标签形式（例如`<br/>`）得到了适当支持，这样的标签后面的文本不会被移除。

值是一个逗号分隔的元素（标签）名称列表，这些元素的内容应被移除。标签名称是不区分大小写的。


<!-- intro -->
##### SQL：

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = \'1\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = \'1\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = \'1\'');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = '1'");
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = '1'");
```

<!-- intro -->
##### Rust：

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

## 从HTML中提取重要部分

### index_sp

<!-- example index_sp -->

```ini
index_sp = {0|1}
```

控制句子和段落边界的检测和索引。可选，默认值为0（不进行检测或索引）。

此指令启用句子和段落边界的检测和索引，使得[SENTENCE](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) 和 [PARAGRAPH](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) 操作符可以工作。句子边界检测基于纯文本分析，只需设置`index_sp = 1`即可启用。段落检测依赖于HTML标记，并在[HTML去除过程](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip)期间进行。因此，要索引段落边界，必须同时设置index_sp指令和html_strip指令为1。

以下规则用于确定句子边界：

* 问号（?）和感叹号（!）总是表示句子的边界。
* 结尾的点（.）表示句子的边界，除非出现以下情况：
    * 后跟一个字母。这被认为是缩写的一部分（例如 "S.T.A.L.K.E.R." 或 "Goldman Sachs S.p.A."）。
    * 后跟一个逗号。这被认为是缩写后跟一个逗号（例如 "Telecom Italia S.p.A., founded in 1994"）。
    * 后跟一个空格和一个小写字母。这被认为是句子中的缩写（例如 "News Corp. announced in February"）。
    * 前跟一个空格和一个大写字母，后跟一个空格。这被认为是中间名（例如 "John D. Doe"）。

段落边界在每个块级HTML标签处检测，包括：ADDRESS, BLOCKQUOTE, CAPTION, CENTER, DD, DIV, DL, DT, H1, H2, H3, H4, H5, LI, MENU, OL, P, PRE, TABLE, TBODY, TD, TFOOT, TH, THEAD, TR, 和 UL。

句子和段落都会使关键词位置计数器增加1。


<!-- intro -->
##### SQL：

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = \'1\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = \'1\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = \'1\'');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = '1'", true);
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = '1'", true);
```

<!-- intro -->
##### Rust：

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

一个字段内的HTML/XML区域列表，用于索引。默认值为空字符串（不会索引任何区域）。

一个“区域”定义为一对匹配的起始和结束标签之间的内容，所有共享相同标签名的span都称为一个“区域”。例如，文档字段中的 `<H1>` 和 `</H1>` 之间的内容属于H1区域。

`index_zones` 指令启用区域索引，但HTML [stripper](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) 也必须启用（通过设置 `html_strip = 1`）。`index_zones` 的值应为逗号分隔的标签名和通配符（以星号结尾）的列表，用于作为区域进行索引。

区域可以嵌套和重叠，只要每个起始标签都有一个匹配的标签。区域也可以用于与 ZONE 运算符匹配，如 [extended_query_syntax](../../Searching/Full_text_matching/Operators.md#ZONE-limit-operator) 中所述。


<!-- intro -->
##### SQL：

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = \'h, th, title\' html_strip = \'1\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = \'h, th, title\' html_strip = \'1\'')
```

<!-- intro -->
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = \'h, th, title\' html_strip = \'1\'');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'", true);
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'", true);
```

<!-- intro -->
##### Rust：

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

