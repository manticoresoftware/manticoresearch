# Advanced HTML tokenization

## Stripping HTML tags

### html_strip

<!-- example html_strip -->

```ini
html_strip = {0|1}
```

Whether to strip HTML markup from incoming full-text data. Optional, default is 0. Known values are 0 (disable stripping) and 1 (enable stripping).

Both HTML tags and entities and considered markup and get processed.

HTML tags are removed, their contents (i.e., everything between `<p>` and `</p>`) are left intact by default. You can choose to keep and index attributes of the tags (e.g., HREF attribute in an A tag, or ALT in an IMG one). Several well-known inline tags are completely removed, all other tags are treated as block level and replaced with whitespace. For example, `te<b>st</b>` text will be indexed as a single keyword  'test', however, `te<p>st</p>` will be indexed as two keywords 'te' and 'st'. Known inline tags are as follows: A, B, I, S, U, BASEFONT,  BIG, EM, FONT, IMG, LABEL, SMALL, SPAN, STRIKE, STRONG, SUB, SUP, TT.

HTML entities get decoded and replaced with corresponding UTF-8 characters. Stripper supports both numeric forms (such as `&#239;`) and  text forms (such as `&oacute;` or `&nbsp;`). All entities as specified by HTML4 standard are supported.

Stripping should work with properly formed HTML and XHTML, but, just as  most browsers, may produce unexpected results on malformed input (such as HTML with stray `<'s` or unclosed `>'s`).

Only the tags themselves, and also HTML comments, are stripped. To strip  the contents of the tags too (eg. to strip embedded scripts), see [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements) option. There are no restrictions on tag names; ie. everything that looks like a valid tag start, or end, or a comment will be stripped.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) html_strip = '1'
```

<!-- request HTTP -->

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

A list of markup attributes to index when stripping HTML. Optional, default is empty (do not index markup attributes).

Specifies HTML markup attributes whose contents should be retained and indexed even though other HTML markup is stripped. The format is per-tag enumeration of indexable attributes, as shown above.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'
```

<!-- request HTTP -->

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

A list of HTML elements for which to strip contents along with the elements themselves. Optional, default is empty string (do not strip contents of any elements).

This feature allows to strip element contents, ie. everything that is between the opening and the closing tags. It is useful to remove embedded scripts, CSS, etc. Short tag form for empty elements (ie. `<br/>`) is properly supported; ie. the text that follows such tag will **not** be removed.

The value is a comma-separated list of element (tag) names whose contents should be removed. Tag names are case insensitive.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'
```

<!-- request HTTP -->

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

Whether to detect and index sentence and paragraph boundaries. Optional, default is 0 (do not detect and index).

This directive enables sentence and paragraph boundary indexing. It's  required for the [SENTENCE](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) and [PARAGRAPH](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) operators to work. Sentence boundary detection is based on plain text analysis, so you only need to set `index_sp = 1` to enable it. Paragraph detection is however based on HTML markup, and happens in the [HTML stripper](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip). So to index paragraph locations you also need to enable the stripper by specifying `html_strip = 1`. Both types of boundaries are detected based on a few built-in rules enumerated just below.

Sentence boundary detection rules are as follows.

* Question and exclamation signs (? and !) are always a sentence boundary.
* Trailing dot (.) is a sentence boundary, except:
    *   When followed by a letter. That's considered a part of an abbreviation (as in "S.T.A.L.K.E.R" or "Goldman Sachs S.p.A.").
    *   When followed by a comma. That's considered an abbreviation followed by a comma (as in "Telecom Italia S.p.A., founded in 1994").
    *   When followed by a space and a small letter. That's considered an abbreviation within a sentence (as in "News Corp. announced in February").
    *   When preceded by a space and a capital letter, and followed by a space. That's considered a middle initial (as in "John D. Doe").

Paragraph boundaries are inserted at every block-level HTML tag. Namely, those are (as taken from HTML 4 standard) ADDRESS, BLOCKQUOTE, CAPTION, CENTER, DD, DIV, DL, DT, H1, H2, H3, H4, H5, LI, MENU, OL, P, PRE, TABLE, TBODY, TD, TFOOT, TH, THEAD, TR, and UL.

Both sentences and paragraphs increment the keyword position counter by 1.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'
```

<!-- request HTTP -->

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

A list of in-field HTML/XML zones to index. Optional, default is empty (do not index zones).

Zones can be formally defined as follows. Everything between an opening and a matching closing tag is called a span, and the aggregate of all spans corresponding sharing the same tag name is called a zone. For instance, everything between the occurrences of `<H1>` and `</H1>` in the document field belongs to H1 zone.

Zone indexing, enabled by `index_zones` directive, is an optional extension of the HTML stripper. So it will also require that the [stripper](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) is enabled (with `html_strip = 1`). The value of the `index_zones` should be a comma-separated list of those tag names and wildcards (ending with a star) that should be indexed as zones.

Zones can nest and overlap arbitrarily. The only requirement is that every opening tag has a matching tag. You can also have an arbitrary number of both zones (as in unique zone names, such as H1) and spans (all the occurrences of those H1 tags) in a document. Once indexed, zones can then be used for matching with the ZONE operator, see [extended_query_syntax](../../Searching/Full_text_matching/Operators.md#ZONE-limit-operator).


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'
```

<!-- request HTTP -->

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
