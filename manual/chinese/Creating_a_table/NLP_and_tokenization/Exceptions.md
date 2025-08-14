# 异常

异常（也称为同义词）允许将一个或多个标记（包括通常会被排除的字符的标记）映射到一个单一的关键词。它们类似于[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)，也执行映射，但有一些重要的区别。

与[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)的区别简要总结如下：

| 异常 | 词形 |
| - | - |
| 区分大小写 | 不区分大小写 |
| 可以使用不在[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)中的特殊字符 | 完全遵守[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) |
| 在大型字典中性能不佳 | 设计用于处理数百万条目 |

## exceptions

```ini
exceptions = path/to/exceptions.txt
```

<!-- example exceptions -->
标记异常文件。可选，默认是空的。
在RT模式下，只允许使用绝对路径。

期望的文件格式是纯文本，每行一个异常。行格式如下：

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

这里的所有标记都区分大小写，并且**不会**被[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)规则处理。因此，以上示例异常文件中，`at&t`文本会被分词为两个关键词`at`和`t`，因为包含小写字母。另一方面，`AT&T`将完全匹配，并生成一个单独的`AT&T`关键词。

如果你需要将`>`或`=`作为普通字符使用，可以通过在它们前面加反斜杠（`\`）进行转义。`>`和`=`都应以这种方式转义。

注意映射到的关键词：
* 总是被解释为*单个*词
* 同时区分大小写和空格敏感

在上面的示例中，`ms windows`查询将*不会*匹配包含`MS Windows`文本的文档。该查询将被解释为两个关键词`ms`和`windows`的查询。`MS Windows`的映射是一个关键词`ms windows`，中间带空格。另一方面，`standartenfuhrer`将检索包含`Standarten Fuhrer`或`Standarten Fuehrer`内容（完全按照此大小写），或关键词本身的任何大小写变体，例如`staNdarTenfUhreR`的文档。（然而它不会匹配`standarten fuhrer`：这个文本由于区分大小写未匹配任何列出的异常，而被索引为两个独立的关键词。）

映射源标记列表中的空白符很重要，但其数量不重要。映射源列表中的任意数量空白符都会匹配索引文档或查询中的任何其他数量的空白符。例如，`AT & T`映射源标记将匹配`AT & T`文本，无论映射源部分和索引文本中的空格数量如何。因此，这类文本会被索引为特殊的`AT&T`关键词，这得益于示例中的第一条目。

异常还允许捕获特殊字符（即一般`charset_table`规则的例外；因此得名）。假设通常你不希望将`+`视为有效字符，但仍然希望能够搜索某些此规则的例外，如`C++`。上述示例正好满足这一需求，与表中的字符无关。

当使用[plain table](../../Creating_a_table/Local_tables/Plain_table.md)时，必须旋转表以合并来自异常文件的更改。在实时表的情况下，更改只会应用于新文档。

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
<!-- proofread -->

