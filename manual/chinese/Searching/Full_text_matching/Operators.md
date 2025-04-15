# 完整文本操作符

查询字符串可以包括特定的操作符，定义如何匹配查询字符串中的单词的条件。

### 布尔操作符

#### AND 操作符

隐式的逻辑 AND 操作符始终存在，因此“hello world”暗示必须在匹配的文档中找到“hello”和“world”两个词。

```sql
hello  world
```

注意：没有显式的 `AND` 操作符。

#### OR 操作符

逻辑 OR 操作符 `|` 的优先级高于 AND，因此 `looking for cat | dog | mouse` 意味着 `looking for (cat | dog | mouse)` 而不是 `(looking for cat) | dog | mouse`。

```sql
hello | world
```

注意：没有操作符 `OR`。请使用 `|` 代替。

### MAYBE 操作符

```sql
hello MAYBE world
```

`MAYBE` 操作符的功能类似于 `|` 操作符，但它不会返回仅匹配右子树表达式的文档。

### 取反操作符

```sql
hello -world
hello !world
```

取反操作符强制规定某个单词不存在的规则。

默认情况下，仅包含 **取反** 的查询 **不** 被支持。要启用，请使用服务器选项 [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed)。

### 字段搜索操作符

```sql
@title hello @body world
```

字段限制操作符将随后的搜索限制在指定字段内。默认情况下，如果给定的字段名不存在于被搜索的表中，则查询将以错误消息失败。然而，通过在查询开始时指定 `@@relaxed` 选项，可以抑制此行为：

```sql
@@relaxed @nosuchfield my query
```

当在具有不同模式的异构表中进行搜索时，这可能会非常有用。

字段位置限制进一步限制搜索到给定字段（或字段）内的前 N 个位置。例如，`@body [50] hello` 将不会匹配关键字 `hello` 在正文中出现在第 51 个位置或更晚的文档。

```sql
@body[50] hello
```

多字段搜索操作符：

```sql
@(title,body) hello world
```

忽略字段搜索操作符（忽略来自 'title' 字段的 'hello world' 的任何匹配）：

```sql
@!title hello world
```

忽略多字段搜索操作符（如果存在字段 'title'、'subject' 和 'body'，则 `@!(title)` 等同于 `@(subject,body)`）：

```sql
@!(title,body) hello world
```

所有字段搜索操作符：

```sql
@* hello
```

### 短语搜索操作符

```sql
"hello world"
```

短语操作符要求单词彼此相邻。

短语搜索操作符可以包括 `match any term` 修饰符。在短语操作符内，术语是位置上重要的。当使用“匹配任何术语”修饰符时，该短语查询中后续术语的位置将发生偏移。因此，“匹配任何”修饰符不影响搜索性能。

```sql
"exact * phrase * * for terms"
```

###  接近搜索操作符

```sql
"hello world"~10
```

接近距离以单词为单位进行计量，考虑到单词计数，并适用于所有在引号内的单词。例如，查询 `"cat dog mouse"~5` 表示必须有少于 8 个包含所有 3 个单词的单词。因此，包含 `CAT aaa bbb ccc DOG eee fff MOUSE` 的文档不会匹配此查询，因为跨度恰好是 8 个单词。

###  半会匹配操作符

```sql
"the world is a wonderful place"/3
```

半会匹配操作符引入了一种模糊匹配的类型。它将仅匹配满足给定阈值的指定单词的文档。在上述示例中（`"the world is a wonderful place"/3`），它将匹配包含至少 6 个指定单词中的 3 个的所有文档。该操作符最多可处理 255 个关键字。您还可以提供一个介于 0.0 和 1.0 之间的值（代表 0% 和 100%），Manticore 将仅匹配包含至少指定百分比的给定单词的文档。上述相同示例也可以表示为 `"the world is a wonderful place"/0.5`，它将匹配包含至少 6 个单词中 50% 的文档。

### 严格顺序操作符

```sql
aaa << bbb << ccc
```

严格顺序操作符（也称为“之前”操作符）仅在其参数关键字按查询中指定的顺序精确出现在文档中时才匹配文档。例如，查询 `black << cat` 会匹配文档 "black and white cat"，但不会匹配文档 "that cat was black"。顺序操作符的优先级最低。它可以应用于单个关键字和更复杂的表达式。例如，这是一个有效的查询：

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### 精确形式修饰符

```sql
raining =cats and =dogs
="exact phrase"
```

精确形式关键字修饰符仅在关键字以指定的精确形式出现时匹配文档。默认情况下，如果词干/词形变化的关键字匹配，则文档被视为匹配。例如，查询 "runs" 将同时匹配包含 "runs" 和包含 "running" 的文档，因为这两种形式都词干为 "run"。然而，`=runs` 查询只会匹配第一个文档。精确形式修饰符需要启用 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 选项。

另一个用例是防止 [扩展](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords) 关键字到它的 `*keyword*` 形式。例如，使用 `index_exact_words=1` + `expand_keywords=1/star`，`bcd` 将找到包含 `abcde` 的文档，但 `=bcd` 不会。
作为影响关键词的修饰符，它可以在短语、接近和法定人数运算符等运算符中使用。对短语运算符应用确切形式修饰符是可能的，在这种情况下，它会将确切形式修饰符内部添加到短语中的所有术语。

### 通配符运算符

```sql
nation* *nation* *national
```

对于前缀（尾部扩展）和/或后缀（头部扩展），需要 [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。如果只希望使用前缀，可以使用 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)。

搜索将尝试查找所有通配符的扩展，每个扩展都被记录为匹配的命中。可以使用 [expansion_limit](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) 表设置控制token的扩展数量。通配符 token 对查询搜索时间可能有重大影响，特别是当 token 长度较短时。在这种情况下，最好使用扩展限制。

如果使用 [expand_keywords](../../Searching/Options.md#expand_keywords) 表设置，可以自动应用通配符运算符。

此外，支持以下内联通配符运算符：

* `?` 可以匹配任何单个字符：`t?st` 将匹配 `test`，但不匹配 `teast`
* `%` 可以匹配零个或一个字符：`tes%` 将匹配 `tes` 或 `test`，但不匹配 `testing`

内联运算符需要 `dict=keywords`（默认启用）和启用前缀/中缀。

### REGEX 运算符

```sql
REGEX(/t.?e/)
```

需要设置 [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 或 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) 和 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords 选项（这是默认值）。

类似于 [通配符运算符](../../Searching/Full_text_matching/Operators.md#Wildcard-operators)，REGEX 运算符尝试查找所有匹配提供的模式的 token，每个扩展都被记录为匹配的命中。请注意，这可能会对查询搜索时间产生重大影响，因为整个字典会被扫描，每个字典中的术语都会与 REGEX 模式进行匹配。

模式应遵循 [RE2 语法](https://github.com/google/re2/wiki/Syntax)。REGEX 表达式分隔符是开放括号后面的第一个符号。换句话说，开放括号后跟分隔符以及分隔符和闭合括号之间的所有文本都被视为 RE2 表达式。
请注意，存储在字典中的术语会经历 `charset_table` 转换，这意味着例如，如果根据 `charset_table`（默认情况下会发生）将所有字符都转为小写，则 REGEX 可能无法匹配大写字符。要使用 REGEX 表达式成功匹配术语，模式必须对应于整个 token。要实现部分匹配，请在模式的开头和/或结尾放置 `.*`。

```sql
REGEX(/.{3}t/)
REGEX(/t.*\d*/)
```

### 字段开始和字段结束修饰符

```sql
^hello world$
```

字段开始和字段结束关键词修饰符确保关键词仅在其出现在完整文本字段的最开始或最后时匹配。例如，查询 `"^hello world$"`（用引号括起来以将短语运算符与开始/结束修饰符结合）将仅匹配包含这两个特定关键词的至少一个字段的文档。

### IDF 提升修饰符

```sql
boosted^1.234 boostedfieldend$^1.234
```

提升修饰符通过所指示的因子提高单词 [IDF](../../Searching/Options.md#idf)_score 在其计算中合并 IDF 的排名分数。它不会以任何方式影响匹配过程。

### NEAR 运算符

```sql
hello NEAR/3 world NEAR/4 "my test"
```

`NEAR` 运算符是接近运算符的更通用版本。其语法为 `NEAR/N`，区分大小写，并且不允许在 `NEAR` 关键字、斜杠和距离值之间有空格。

虽然原始接近运算符仅在关键词集合上工作，但 `NEAR` 更为灵活，接受任意子表达式作为其两个参数。当两个子表达式在 N 个单词内同时找到时，它匹配文档，无论其顺序如何。`NEAR` 是左结合的，具有与 [BEFORE](../../Searching/Full_text_matching/Operators.md#Strict-order-operator) 相同的（最低）优先级。

重要的是要注意，`one NEAR/7 two NEAR/7 three` 并不完全等价于 `"one two three"~7`。关键区别在于，接近运算符允许在所有三个匹配词之间有最多 6 个不匹配的单词，而带有 `NEAR` 的版本限制较少：它允许在 `one` 和 `two` 之间有最多 6 个单词，然后再在那两个词之间与 `three` 之间有最多 6 个单词。

### NOTNEAR 运算符

```sql
Church NOTNEAR/3 street
```
`NOTNEAR` 运算符作为否定断言。它在左侧参数存在且右侧参数在文档中缺失或右侧参数距离左边匹配参数的末尾有指定距离时匹配文档。距离以单词表示。语法为 `NOTNEAR/N`，区分大小写，并且不允许在 `NOTNEAR` 关键字、斜杠和距离值之间有空格。该运算符的两个参数可以是术语或任何运算符或运算符组。

### SENTENCE 和 PARAGRAPH 运算符

```sql
all SENTENCE words SENTENCE "in one sentence"
```


```sql
"Bill Gates" PARAGRAPH "Steve Jobs"
```
`SENTENCE` 和 `PARAGRAPH` 操作符在它们两个的参数分别位于同一句话或同一段文本时匹配文档。这些参数可以是关键字、短语，或同一操作符的实例。

参数在句子或段落中的顺序无关紧要。这些操作符只在启用了 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)（句子和段落索引功能）的表中有效，其他情况下则回退为简单的与操作。有关句子和段落的构成信息，请参阅 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp) 指令文档。


### ZONE 限制操作符

```sql
ZONE:(h3,h4)

仅在这些标题中
```

`ZONE limit` 操作符与字段限制操作符非常相似，但将匹配限制在指定的字段区域或区域列表中。需要注意的是，后续子表达式不需要在给定区域的单个连续跨度内匹配，可能会跨多个跨度匹配。例如，查询 `(ZONE:th hello world)` 将匹配以下示例文档：

```html
<th>表 1. 对 Hello Kitty 品牌的地方认知。</th>
.. 这里有一些表数据 ..
<th>表 2. 全球品牌认知。</th>
```

`ZONE` 操作符在下一个字段或 `ZONE` 限制操作符之前，或在闭合括号之前影响查询。它仅对支持区域的表有效（请参阅 [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)），否则将被忽略。

### ZONESPAN 限制操作符

```sql
ZONESPAN:(h2)

仅在（单个）标题中
```

`ZONESPAN` 限制操作符类似于 `ZONE` 操作符，但要求匹配发生在单个连续跨度内。在之前提供的示例中，`ZONESPAN:th hello world` 将不会匹配文档，因为“hello”和“world”并未出现在同一个跨度内。

<!-- proofread -->

