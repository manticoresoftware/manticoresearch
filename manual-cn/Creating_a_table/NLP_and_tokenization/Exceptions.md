# 异常处理

异常处理（也称为同义词）允许将一个或多个标记（包括通常被排除的特殊字符）映射到单一关键词。这与[词形形式](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)类似，因为它们也执行映射操作，但两者之间有一些重要区别。

以下是与[词形形式](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)的主要区别：

| 异常处理                                                     | 词形形式                                                     |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| 区分大小写                                                   | 不区分大小写                                                 |
| 可以使用不在[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)中的特殊字符 | 完全遵循[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) |
| 在处理大型字典时表现较差                                     | 设计用于处理数百万条记录                                     |

## exceptions

```ini
exceptions = path/to/exceptions.txt
```

<!-- example exceptions -->

标记化异常文件。可选，默认值为空。在实时模式下，仅允许使用绝对路径。

预期文件格式为纯文本，每行一个异常，行格式如下：

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

这里所有的标记都是区分大小写的，并且**不会**通过[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)规则进行处理。因此，使用上面的示例异常文件，`at&t`文本会被标记为两个关键词 `at` 和 `t`，因为字母是小写的。另一方面，`AT&T`将完全匹配并生成一个单一的 `AT&T`关键词。

如果需要使用 `>` 或 `=` 作为普通字符，可以通过在每个字符前加反斜杠 (`\`) 来转义它们。`>` 和 `=` 都应以这种方式进行转义。

注意，映射到的关键词：

- 始终被解释为一个**单词**
- 区分大小写和空格

在上述示例中，`ms windows`查询**不会**匹配含有 `MS Windows` 文本的文档。查询将被解释为两个关键词，`ms` 和 `windows`。而 `MS Windows` 的映射是一个包含空格的关键词 `ms windows`。另一方面，`standartenfuhrer`将能够检索到包含 `Standarten Fuhrer` 或 `Standarten Fuehrer` 内容的文档（准确的大小写），或者匹配关键词本身的任何大小写变体，如 `staNdarTenfUhreR`。（然而，它不会匹配 `standarten fuhrer`，因为此文本不符合列表中的任何异常，由于大小写敏感性，它会被索引为两个独立的关键词。）

映射中的空格数量很重要，但其数量无关紧要。映射列表中的任何数量的空格都将匹配索引文档或查询中的任何数量的空格。例如，`AT & T` 映射将匹配 `AT & T` 文本，无论两者中的空格数量是多少。这样的文本将因此被索引为一个特殊的 `AT&T` 关键词，感谢示例中的第一条条目。

异常处理还允许捕获特殊字符（即偏离一般 `charset_table` 规则的字符，这也是名称的由来）。假设你通常不希望将 `+` 视为有效字符，但仍然希望能够搜索类似 `C++` 的例外情况。上面的示例正是这样做的，完全独立于表中的字符设置。

当使用[普通表](../../Creating_a_table/Local_tables/Plain_table.md)时，必须旋转表以应用异常文件中的更改。对于实时表，变更只会应用于新文档。

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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'");
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
