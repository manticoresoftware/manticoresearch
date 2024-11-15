# 全文搜索操作符

查询字符串可以包含特定的操作符，这些操作符定义了查询字符串中的单词应如何匹配的条件。

### 布尔操作符

#### AND 操作符

一个隐式的逻辑 AND 操作符总是存在，所以 "hello world" 意味着匹配的文档必须同时包含 "hello" 和 "world"。

```sql
hello  world
```

注意：没有显式的 `AND` 操作符。

#### OR 操作符

逻辑 OR 操作符 | 的优先级高于 AND，所以 `looking for cat | dog | mouse` 表示 `looking for (cat | dog | mouse)` 而不是 `(looking for cat) | dog | mouse`。

```sql
hello | world
```

注意：没有 `OR` 操作符，请使用 `|` 代替。

### MAYBE 操作符

```sql
hello MAYBE world
```

`MAYBE` 操作符类似于 `|` 操作符，但它不会返回仅匹配右侧子表达式的文档。

### 否定操作符

```sql
hello -world
hello !world
```

否定操作符强制规则，即某个单词不应存在。

**仅**包含否定操作符的查询默认**不**支持。若要启用此功能，请使用服务器选项 [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed)。

### 字段搜索操作符

```sql
@title hello @body world
```

字段限制操作符将后续搜索限制在指定字段中。默认情况下，如果给定字段名在搜索的表中不存在，查询将失败并返回错误信息。然而，这种行为可以通过在查询开头指定 `@@relaxed` 选项来抑制：

```sql
@@relaxed @nosuchfield my query
```

这在通过不同模式的异构表进行搜索时非常有用。

字段位置限制进一步将搜索限制在给定字段的前 N 个位置。例如，`@body [50] hello` 将不会匹配那些关键词 hello 出现在 body 字段中的第 51 位或更后的文档。

```sql
@body[50] hello
```

多字段搜索操作符：

```sql
@(title,body) hello world
```

忽略字段搜索操作符（忽略来自 ‘title’ 字段的 ‘hello world’ 匹配）：

```sql
@!title hello world
```

忽略多字段搜索操作符（如果有字段 ‘title’，‘subject’，‘body’，则 `@!(title)` 等价于 `@(subject,body)`）：

```sql
@!(title,body) hello world
```

全字段搜索操作符：

```sql
@* hello
```

### 短语搜索操作符

```sql
"hello world"
```

短语操作符要求单词彼此相邻。

短语搜索操作符可以包含一个 “匹配任意词” 修饰符。在短语操作符内，词语的位置很重要。当使用 “匹配任意词” 修饰符时，短语查询中的后续词语的位置将被移动。因此，“匹配任意词” 修饰符不会影响搜索性能。

```sql
"exact * phrase * * for terms"
```

###  近邻搜索操作符

```sql
"hello world"~10
```

近邻距离以词数来衡量，适用于引号内的所有词语。例如，查询 `"cat dog mouse"`~5 表示必须有少于 8 个词语的范围包含所有 3 个词。因此，一个包含 `CAT aaa bbb ccc DOG eee fff MOUSE` 的文档不会匹配此查询，因为该范围恰好是 8 个词长。

###  多数匹配操作符

```sql
"the world is a wonderful place"/3
```

多数匹配操作符引入了一种模糊匹配类型。它只会匹配满足指定词语数量阈值的文档。在上述示例 (`"the world is a wonderful place"/3`) 中，它将匹配包含至少 6 个指定词中 3 个的文档。该操作符的限制是最多 255 个关键词。你还可以提供 0.0 到 1.0 之间的值（分别表示 0% 和 100%），Manticore 仅匹配包含至少指定百分比词语的文档。同样的例子也可以表示为 `"the world is a wonderful place"/0.5`，它将匹配至少包含 6 个词中 50% 的文档。

### 严格顺序操作符

```sql
aaa << bbb << ccc
```

严格顺序操作符（也称为 "之前" 操作符）仅在其参数关键词出现在文档中的顺序与查询中指定的顺序完全一致时才匹配文档。例如，查询 `black << cat` 将匹配文档 "black and white cat"，但不会匹配文档 "that cat was black"。顺序操作符优先级最低，它可以应用于单个关键词以及更复杂的表达式。例如，这是一个有效查询：

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### 精确形式修饰符

```sql
raining =cats and =dogs
="exact phrase"
```

精确形式关键词修饰符仅在文档中的关键词与指定的精确形式一致时才匹配文档。默认情况下，文档在关键词的词干/词形化形式匹配时视为匹配。例如，查询 "runs" 将匹配包含 "runs" 和 "running" 的文档，因为这两种形式的词干都为 "run"。然而，查询 `=runs` 只会匹配包含 "runs" 的文档。精确形式操作符需要启用 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 选项。

另一个使用场景是防止 [扩展](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords) 关键词为 `*keyword*` 形式。例如，使用 `index_exact_words=1` 和 `expand_keywords=1/star`，`bcd` 会找到包含 `abcde` 的文档，但 `=bcd` 不会。

作为影响关键词的修饰符，它可以在操作符中使用，例如短语、近邻和多数匹配操作符。将精确形式修饰符应用于短语操作符时，它会自动将该修饰符添加到短语中的所有词语上。

### 通配符操作符

```sql
nation* *nation* *national
```

需要 [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 用于前缀（尾部扩展）和/或后缀（首部扩展）。如果只需要前缀，[min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) 也可使用。

搜索将尝试找到通配符标记的所有扩展，并将每个扩展记录为一个匹配结果。可以通过 [expansion_limit](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) 表设置控制标记的扩展数量。通配符标记可能对查询时间有显著影响，尤其是标记长度较短时。在这种情况下，使用扩展限制是有利的。

如果使用 [expand_keywords](../../Searching/Options.md#expand_keywords) 表设置，通配符操作符可自动应用。

此外，支持以下内联通配符操作符：

- `?` 匹配任意单个字符：`t?st` 将匹配 `test`，但不匹配 `teast`
- `%` 匹配零个或一个字符：`tes%` 将匹配 `tes` 或 `test`，但不匹配 `testing`

内联操作符需要 `dict=keywords` 和已启用的 infixing。

### 正则表达式操作符

```sql
REGEX(/t.?e/)
```

需要设置 [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 或 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) 和 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords（这是默认设置）。

类似于 [通配符操作符](../../Searching/Full_text_matching/Operators.md#通配符操作符)，正则表达式操作符尝试找到所有与提供的模式匹配的标记，并将每个扩展记录为一个匹配结果。请注意，这可能对查询时间有显著影响，因为整个字典会被扫描，每个词条都会与正则表达式模式进行匹配。

这些模式应符合 [RE2 语法](https://github.com/google/re2/wiki/Syntax)。正则表达式分隔符是左括号后的第一个符号。换句话说，所有文本位于左括号后紧跟分隔符和右括号之间，都被视为 RE2 表达式。 请注意，存储在字典中的词条会经过 `charset_table` 转换，这意味着例如如果根据 `charset_table` 的设置所有字符都被转为小写（默认情况下会发生这种情况），则正则表达式可能无法匹配大写字符。若要通过正则表达式成功匹配某个词条，模式必须与整个标记一致。要实现部分匹配，请在模式的开头和/或结尾添加 `.*`。

```sql
REGEX(/.{3}t/)
REGEX(/t.*\d*/)
```

### 字段开头与字段结尾修饰符

```sql
^hello world$
```

字段开头与字段结尾关键词修饰符确保关键词仅在它出现在全文字段的最开头或最结尾时匹配。例如，查询 `"^hello world$"`（使用引号将短语操作符与开头/结尾修饰符结合）将仅匹配至少有一个字段包含这两个特定关键词的文档。

### IDF boost modifier

```sql
boosted^1.234 boostedfieldend$^1.234
```

提升修饰符通过在包含 IDF 的计算中按指定系数提升单词 [IDF](../../Searching/Options.md#idf)_score。它不会以任何方式影响匹配过程。

### NEAR 操作符

```sql
hello NEAR/3 world NEAR/4 "my test"
```

`NEAR` 操作符是一个更通用的近邻操作符。其语法为 `NEAR/N`，区分大小写，不允许在 `NEAR` 关键词、斜杠符号和距离值之间有空格。

虽然原始的近邻操作符仅适用于关键词集，但 `NEAR` 更加灵活，它可以接受任意子表达式作为其两个参数。当这两个子表达式在 N 个词内出现时，`NEAR` 就会匹配文档，无论它们的顺序如何。`NEAR` 是左结合的，并且具有与 [BEFORE](../../Searching/Full_text_matching/Operators.md#严格顺序操作符) 相同的（最低）优先级。

需要注意的是，`one NEAR/7 two NEAR/7 three` 并不完全等同于 `"one two three"~7`。关键区别在于，近邻操作符允许在三个匹配词之间最多出现 6 个不匹配词，而使用 `NEAR` 的版本则更不严格：它允许 `one` 和 `two` 之间最多有 6 个词，然后 `two` 和 `three` 之间再允许最多 6 个词。

### NOTNEAR 操作符

```sql
Church NOTNEAR/3 street
```
`NOTNEAR` 操作符作为一种否定断言。当左参数存在，并且右参数要么不存在于文档中，要么与左匹配参数末尾相隔指定距离时，它会匹配文档。距离以词语为单位表示。语法为 `NOTNEAR/N`，区分大小写，并且不允许在 `NOTNEAR` 关键词、斜杠符号和距离值之间有空格。该操作符的两个参数可以是词语、任意操作符或操作符组。

### SENTENCE 和 PARAGRAPH 操作符

```sql
all SENTENCE words SENTENCE "in one sentence"
```


```sql
"Bill Gates" PARAGRAPH "Steve Jobs"
```
`SENTENCE` 和 `PARAGRAPH` 操作符在其两个参数位于同一段落或同一句话中时匹配文档。这些参数可以是关键词、短语或相同操作符的实例。

在句子或段落中的参数顺序无关紧要。这些操作符仅在使用启用了 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)（句子和段落索引功能）构建的表时有效，否则会回退为简单的 AND 操作。有关句子和段落的定义，请参考 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp) 指令文档。


### ZONE 限制操作符

```sql
ZONE:(h3,h4)

only in these titles
```

`ZONE 限制` 操作符与字段限制操作符相似，但将匹配限制在指定的字段区域或区域列表中。需要注意的是，后续子表达式不需要在给定区域的单个连续跨度内匹配，可以跨越多个跨度匹配。例如，查询 `(ZONE:th hello world)` 将匹配以下示例文档：

```html
<th>Table 1. Local awareness of Hello Kitty brand.</th>
.. some table data goes here ..
<th>Table 2. World-wide brand awareness.</th>
```

`ZONE` 操作符影响查询，直到下一个字段或 `ZONE` 限制操作符，或直到关闭的括号。它仅在使用启用区域支持（请参考 [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)）构建的表时有效，否则将被忽略。

### ZONESPAN 限制操作符

```sql
ZONESPAN:(h2)

only in a (single) title
```

`ZONESPAN` 限制操作符类似于 `ZONE` 操作符，但要求匹配发生在单个连续跨度内。在前面的示例中，`ZONESPAN:th hello world` 不会匹配文档，因为 "hello" 和 "world" 没有出现在同一个跨度内。

<!-- proofread -->
