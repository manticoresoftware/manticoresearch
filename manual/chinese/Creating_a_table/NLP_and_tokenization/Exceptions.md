# 异常

异常（也称为同义词）允许将一个或多个标记（包括通常会被排除的字符标记）映射到单个关键词。它们类似于[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)，也执行映射操作，但有一些重要的区别。

与[wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)的区别简要总结如下：

| 异常 | 单词形式 |
| - | - |
| 区分大小写 | 不区分大小写 |
| 可以使用不在[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)中的特殊字符 | 完全遵守[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) |
| 在庞大字典中表现较差 | 设计用于处理数百万条目 |

## exceptions

```ini
exceptions = path/to/exceptions.txt
```

<!-- example exceptions -->
分词异常文件。可选，默认为空。
在RT模式下，只允许使用绝对路径。

期望的文件格式是纯文本，异常一行一个。行格式如下：

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

这里的所有标记都区分大小写且**不会**被[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)规则处理。因此，使用上述示例异常文件，文本`at&t`会因小写字母而被分为两个关键词`at`和`t`。另一方面，`AT&T`将被精确匹配并生成单一的`AT&T`关键词。

如果需要将`>`或`=`作为普通字符使用，可以在每个字符前加反斜杠(`\`)进行转义。`>`和`=`都应如此转义。

注意映射到的关键词：
* 总是被解释为*单个*词
* 同时区分大小写和空格

在上述示例中，查询`ms windows`不会匹配包含文本`MS Windows`的文档。该查询被解释为两个关键词的查询，`ms`和`windows`。映射`MS Windows`则是包含空格的单一关键词`ms windows`。另一方面，`standartenfuhrer`将检索包含`Standarten Fuhrer`或`Standarten Fuehrer`内容（大小写完全如示例所示）或关键词本身任意大小写变体的文档，例如`staNdarTenfUhreR`。（但不会捕捉`standarten fuhrer`，因为此文本因区分大小写不符合任何列出的异常，会被索引为两个独立关键词。）

映射来源的标记列表中空白字符是重要的，但其数量不重要。映射列表中的任意数量空白都能匹配文档或查询中任意数量的空白。例如，`AT & T`映射源标记将匹配文本`AT & T`，无论映射源部分和索引文本中的空格量多少。因此，这样的文本将被索引为特殊的`AT&T`关键词，依赖示例中的第一条条目。

异常还允许捕获特殊字符（这些字符一般不符合`charset_table`规则，因此得名“异常”）。假设您通常不希望将`+`视为有效字符，但又想能够搜索如`C++`的某些例外，上述示例正好实现了这一点，完全独立于字符表中包含何种字符。

使用[plain table](../../Creating_a_table/Local_tables/Plain_table.md)时，必须旋转表以合并异常文件的更改。对于实时表，更改仅适用于新文档。

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

