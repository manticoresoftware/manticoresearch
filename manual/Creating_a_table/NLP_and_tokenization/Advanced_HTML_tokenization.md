# Advanced HTML tokenization

## Stripping HTML tags

### html_strip

<!-- example html_strip -->

```ini
html_strip = {0|1}
```

This option determines whether HTML markup should be stripped from the incoming full-text data. The default value is 0, which disables stripping. To enable stripping, set the value to 1.

HTML tags and entities are considered as markup and will be processed. 

HTML tags are removed, while the contents between them (e.g. everything between `<p>` and `</p>`) are left intact. You can choose to keep and index tag attributes (e.g. HREF attribute in an A tag or ALT in an IMG tag). Some well-known inline tags, such as A, B, I, S, U, BASEFONT, BIG, EM, FONT, IMG, LABEL, SMALL, SPAN, STRIKE, STRONG, SUB, SUP, and TT, are completely removed. All other tags are treated as block level and are replaced with whitespace. For example, the text `te<b>st</b>` will be indexed as a single keyword 'test', while `te<p>st</p>` will be indexed as two keywords 'te' and 'st'.

HTML entities are decoded and replaced with their corresponding UTF-8 characters. The stripper supports both numeric forms (e.g. `&#239;`) and text forms (e.g. `&oacute;` or `&nbsp;`) of entities, and supports all entities specified by the HTML4 standard.

The stripper is designed to work with properly formed HTML and XHTML, but may produce unexpected results on malformed input (such as HTML with stray `<'s` or unclosed `>'s`).

Please note that only the tags themselves, as well as HTML comments, are stripped. To strip the contents of the tags, including embedded scripts, see the [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements) option. There are no restrictions on tag names, meaning that everything that looks like a valid tag start, end, or comment will be stripped.


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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_strip = '1'");
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

The html_index_attrs option allows you to specify which HTML markup attributes should be indexed even though other HTML markup is stripped. The default value is empty, meaning no attributes will be indexed. 
The format of the option is a per-tag enumeration of indexable attributes, as demonstrated in the example above. The contents of the specified attributes will be retained and indexed, providing a way to extract additional information from your full-text data.


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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = '1'");
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

This option allows you to remove the contents of elements, meaning everything between the opening and closing tags. It is useful for removing embedded scripts, CSS, etc. The short tag form for empty elements (e.g. <br/>) is properly supported, and the text following such a tag will not be removed.

The value is a comma-separated list of element (tag) names, the contents of which should be removed. Tag names are case-insensitive.


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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = '1'");
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

This directive enables the detection and indexing of sentence and paragraph boundaries, making it possible for the [SENTENCE](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators)  and [PARAGRAPH](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators)  operators to work. Sentence boundary detection is based on plain text analysis, and only requires setting `index_sp = 1` to enable it. Paragraph detection, however, relies on HTML markup and occurs during the [HTML stripping process](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip. As such, to index paragraph boundaries, both the index_sp directive and the html_strip directive must be set to 1.

The following rules are used to determine sentence boundaries:

* Question marks (?) and exclamation marks (!) always indicate a sentence boundary. 
* Trailing dots (.) indicate a sentence boundary, except in the following cases:
    * When followed by a letter. This is considered part of an abbreviation (e.g. "S.T.A.L.K.E.R." or "Goldman Sachs S.p.A.").
    * When followed by a comma. This is considered an abbreviation followed by a comma (e.g. "Telecom Italia S.p.A., founded in 1994").
    * When followed by a space and a lowercase letter. This is considered an abbreviation within a sentence (e.g. "News Corp. announced in February").
    * When preceded by a space and an uppercase letter, and followed by a space. This is considered a middle initial (e.g. "John D. Doe").
        
Paragraph boundaries are detected at every block-level HTML tag, including: ADDRESS, BLOCKQUOTE, CAPTION, CENTER, DD, DIV, DL, DT, H1, H2, H3, H4, H5, LI, MENU, OL, P, PRE, TABLE, TBODY, TD, TFOOT, TH, THEAD, TR, and UL.

Both sentences and paragraphs increment the keyword position counter by 1.


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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = '1'");
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

A list of HTML/XML zones within a field to be indexed. The default is an empty string (no zones will be indexed).

A "zone" is defined as everything between an opening and a matching closing tag, and all spans sharing the same tag name are referred to as a "zone." For example, everything between `<H1>` and `</H1>` in a document field belongs to the H1 zone.

The `index_zones` directive enables zone indexing, but the HTML [stripper](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) must also be enabled (by setting `html_strip = 1`). The value of `index_zones` should be a comma-separated list of tag names and wildcards (ending with a star) to be indexed as zones.

Zones can be nested and overlap, as long as every opening tag has a matching tag. Zones can also be used for matching with the ZONE operator, as described in the [extended_query_syntax](../../Searching/Full_text_matching/Operators.md#ZONE-limit-operator).


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
##### javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = \'h, th, title\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'");
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