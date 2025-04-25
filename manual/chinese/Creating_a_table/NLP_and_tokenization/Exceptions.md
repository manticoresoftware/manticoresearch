# 例外

例外（也称为同义词）允许将一个或多个令牌（包括通常会被排除的字符的令牌）映射到一个单一的关键词。它们与 [词形](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) 类似，因为它们也执行映射，但有一些重要的区别。

与 [词形](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) 的区别简要总结如下：

| 例外 | 词形 |
| - | - |
| 区分大小写 | 不区分大小写 |
| 可以使用不在 [字符集表](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 中的特殊字符 | 完全遵循 [字符集表](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) |
| 在大型字典中表现不佳 | 设计为处理数百万条记录 |

## exceptions

```ini
exceptions = path/to/exceptions.txt
```

<!-- example exceptions -->
解析例外文件。可选，默认值为空。
在 RT 模式下，仅允许绝对路径。

期待的文件格式是纯文本，每个例外一行。行格式如下：

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
=>abc> => abc
```

这里的所有令牌都区分大小写，并且将 **不会** 被 [字符集表](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 规则处理。因此，使用上面的示例例外文件，`at&t` 文本将被标记为两个关键词 `at` 和 `t`，由于小写字母。另一方面，`AT&T` 将完全匹配，并生成一个单一的 `AT&T` 关键词。

如果您需要将 `>` 或 `=` 视为常规字符，可以通过在每个字符前加反斜杠（``）来转义它们。`>` 和 `=` 都应以这种方式转义。

请注意，映射到的关键词：
* 始终被解释为一个 *单一* 单词
* 都区分大小写和空格

在上面的示例中，`ms windows` 查询将 *不* 匹配包含 `MS Windows` 文本的文档。该查询将被解释为针对两个关键词的查询，`ms` 和 `windows`。对 `MS Windows` 的映射是一个单一的关键词 `ms windows`，中间有一个空格。另一方面，`standartenfuhrer` 将检索包含 `Standarten Fuhrer` 或 `Standarten Fuehrer` 内容的文档（确切地以这种方式大写），或关键词本身的任何大小写变体，例如 `staNdarTenfUhreR`。 （但是它不会匹配 `standarten fuhrer`，因为该文本不符合任何列出的例外，因为区分大小写，并作为两个单独的关键词被索引。）

map-from 令牌列表中的空白是重要的，但其数量无关紧要。map-form 列表中的任何数量的空白都将匹配索引文档或查询中的任何其他数量的空白。例如，`AT & T` map-from 令牌将匹配 `AT & T` 文本，不论在 map-from 部分和索引文本中的空格有多少。因此，这样的文本将被索引为特定的 `AT&T` 关键词，感谢示例中的第一个条目。

例外还允许捕获特殊字符（这些字符是一般 `字符集表` 规则的例外，因此称为例外）。假设您通常不想将 `+` 视为有效字符，但仍然希望能够搜索此规则的一些例外，例如 `C++`。上面的示例正好可以做到这一点，与表中包含的字符及其不包含的字符完全独立。

使用 [纯表](../../Creating_a_table/Local_tables/Plain_table.md) 时，必须旋转表以纳入来自例外文件的更改。在实时表的情况下，更改仅适用于新文档。

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
$index = new ManticoresearchIndex($client);
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
utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'');
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

# 示例

示例（也称为同义词）允许将一个或多个标记（包括通常会被排除的字符的标记）映射到单个关键字。它们与[词形](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)类似，因为它们也执行映射，但有许多重要的差异。

与[词形](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)的差异的简要总结如下：

| 示例 | 词形 |
| - | - |
| 大小写敏感 | 大小写不敏感 |
| 可以使用不在[字符集表](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)中的特殊字符 | 完全遵循[字符集表](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) |
| 在巨大的字典上表现不佳 | 设计用于处理数百万条目 |

## 示例

```ini
exceptions = path/to/exceptions.txt
```

<!-- 示例异常 -->
标记化异常文件。可选，默认为空。
在 RT 模式下，仅允许绝对路径。

预期的文件格式是纯文本，每个异常一行。行格式如下：

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

此处的所有标记均为大小写敏感，并且**不**会被[字符集表](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)规则处理。因此，在上面的示例异常文件中，`at&t`文本将被标记化为两个关键字`at`和`t`，因为使用了小写字母。另一方面，`AT&T`将完全匹配并生成一个单独的`AT&T`关键字。

如果您需要将`>`或`=`作为普通字符使用，可以通过在每个字符前加反斜杠（`\`）对其进行转义。这两者应以这种方式进行转义。

请注意，映射到的关键字：
* 始终被解释为一个*单一*单词
* 都对大小写和空格敏感

在上述示例中，`ms windows`查询将*不*匹配包含`MS Windows`文本的文档。该查询将被解释为对两个关键字`ms`和`windows`的查询。对于`MS Windows`的映射是一个单一的关键字`ms windows`，中间有一个空格。另一方面，`standartenfuhrer`将检索包含`Standarten Fuhrer`或`Standarten Fuehrer`内容的文档（完全大写形式，如此），或关键字本身的任何大小写变体，例如`staNdarTenfUhreR`。（不过，它不会捕捉`standarten fuhrer`，因为该文本由于大小写敏感而不匹配任何列出的异常，且被索引为两个单独的关键字。）

映射到的标记列表中的空格很重要，但其数量并不重要。映射到的标记列表中的任何数量的空白都将与已索引文档或查询中的任意数量的空白匹配。例如，`AT & T`映射到的标记将匹配`AT & T`文本，无论映射到部分和已索引文本中的空格数量。因此，该文本将被索引为一个特殊的`AT&T`关键字，这要归功于示例中的第一个条目。

示例还允许捕捉特殊字符（这些字符是一般`字符集表`规则的例外，因此有此名称）。假设您通常不想将`+`视为有效字符，但仍希望能够搜索一些例外，例如`C++`。上面的示例将完全独立于表中的字符以及非字符的情况。

在使用[纯表](../../Creating_a_table/Local_tables/Plain_table.md)时，必须旋转表以纳入来自异常文件的更改。在实时表的情况下，更改仅适用于新文档。

<!-- 请求 SQL -->

```sql
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'
```

<!-- 请求 JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'"
```

<!-- 请求 PHP -->

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
<!-- 介绍 -->
##### Python:

<!-- 请求 Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'')
```
<!-- 介绍 -->
##### Javascript:

<!-- 请求 javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'');
```

<!-- 介绍 -->
##### Java:
<!-- 请求 Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'");
```

<!-- 介绍 -->
##### C#:
<!-- 请求 C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'");
```

<!-- 请求 CONFIG -->

```ini
table products {
  exceptions = /usr/local/manticore/data/exceptions.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- 结束 -->
<!-- 校对 -->
