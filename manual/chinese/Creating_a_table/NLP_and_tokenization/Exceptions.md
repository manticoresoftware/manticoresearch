# 异常

异常（也称为同义词）允许将一个或多个标记（包括通常会被排除的字符的标记）映射到单个关键字。它们与[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)类似，因为它们也执行映射，但有一些重要的区别。

与[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)的区别简要总结如下：

| 异常 | 词形 |
| - | - |
| 区分大小写 | 不区分大小写 |
| 可以使用不在[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)中的特殊字符 | 完全遵守[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) |
| 在大型词典中性能较差 | 设计用于处理数百万条目 |

## exceptions

```ini
exceptions = path/to/exceptions.txt
```

<!-- example exceptions -->
对异常文件进行分词。可选，默认为空。
在RT模式下，仅允许使用绝对路径。

预期的文件格式是纯文本，每行一个异常。行格式如下：

```ini
map-from-tokens => map-to-token
```

示例文件：

```ini
at & t => at&t
AT&T => AT&T
Standarten   Fuehrer => standartenfuhrer
Standarten Fuhrer => standartenfuhrer
MS Windows => ms windows
Microsoft Windows => ms windows
C++ => cplusplus
c++ => cplusplus
C plus plus => cplusplus
\=\>abc\> => abc
```

此处的所有标记都是区分大小写的，不会被[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)规则处理。因此，使用上面的示例异常文件，`at&t`文本将被分词为两个关键字`at`和`t`，因为是小写字母。另一方面，`AT&T`将完全匹配并生成一个`AT&T`关键字。

如果需要将`>`或`=`作为普通字符使用，可以通过在每个字符前加上反斜杠(`\`)进行转义。这两种字符都应以这种方式转义。

请注意映射到的关键字：
* 总是被解释为一个*单个*词
* 同时区分大小写和空格

在上面的示例中，`ms windows`查询将*不*匹配包含`MS Windows`文本的文档。该查询将被解释为对两个关键字`ms`和`windows`的查询。`MS Windows`的映射是一个包含空格的单个关键字`ms windows`。另一方面，`standartenfuhrer`将检索包含`Standarten Fuhrer`或`Standarten Fuehrer`内容的文档（完全按照此方式大写），或关键字本身的任何大小写变体，例如`staNdarTenfUhreR`。（但是不会匹配`standarten fuhrer`：由于大小写敏感，此文本不匹配任何列出的异常，并被索引为两个独立的关键字。）

映射来源标记列表中的空格很重要，但其数量不重要。映射形式列表中的任何数量的空格都将匹配索引文档或查询中的任何其他数量的空格。例如，`AT & T`映射来源标记将匹配`AT & T`文本，无论映射来源部分和索引文本中的空格数量如何。因此，此类文本将被索引为一个特殊的`AT&T`关键字，这要归功于示例中的第一个条目。

异常还允许捕获特殊字符（这些字符是通用`charset_table`规则的例外；因此得名）。假设通常不希望将`+`视为有效字符，但仍希望能够搜索某些例外情况，例如`C++`。上面的示例将实现这一点，完全独立于表中包含哪些字符以及不包含哪些字符。

当使用[plain table](../../Creating_a_table/Local_tables/Plain_table.md)时，需要旋转表以合并异常文件中的更改。在实时表的情况下，更改仅适用于新文档。

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'exceptions' => '/usr/local/manticore/data/exceptions.txt'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  exceptions = /usr/local/manticore/data/exceptions.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## exceptions_list

```ini
exceptions_list = 'map-from-tokens => map-to-token; ...'
```

<!-- example exceptions_list -->
`exceptions_list`设置允许您在`CREATE TABLE`语句中直接指定异常。仅在[RT模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)中支持。

值必须用分号(`;`)分隔。由于异常可能包含`>`或`=>`作为分隔符，并且可能包含其他特殊字符，请确保如果分号是标记本身的一部分，则转义分号（例如`\;`）。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) exceptions_list = 'at & t => at&t; MS Windows => ms windows'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) exceptions_list = 'at & t => at&t; MS Windows => ms windows'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'exceptions_list' => 'at & t => at&t; MS Windows => ms windows'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) exceptions_list = \'at & t => at&t; MS Windows => ms windows\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions_list = \'at & t => at&t; MS Windows => ms windows\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions_list = \'at & t => at&t; MS Windows => ms windows\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) exceptions_list = 'at & t => at&t; MS Windows => ms windows'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) exceptions_list = 'at & t => at&t; MS Windows => ms windows'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) exceptions_list = 'at & t => at&t; MS Windows => ms windows'", Some(true)).await;
```

<!-- end -->
<!-- proofread -->

