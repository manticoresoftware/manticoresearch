# 完全文本操作符

查询字符串可以包含特定的操作符，用于定义查询字符串中的单词应如何匹配的条件。

### 布尔操作符

#### AND 操作符

隐式的逻辑 AND 操作符始终存在，因此“hello world”意味着匹配的文档中必须同时包含“hello”和“world”。

```sql
hello  world
```

注意：没有显式的 `AND` 操作符。

#### OR 操作符

逻辑 OR 操作符 `|` 的优先级高于 AND，因此 `looking for cat | dog | mouse` 表示 `looking for (cat | dog | mouse)`，而不是 `(looking for cat) | dog | mouse`。

```sql
hello | world
```

注意：没有 `OR` 操作符。请使用 `|` 代替。

### MAYBE 操作符

```sql
hello MAYBE world
```

`MAYBE` 操作符的功能类似于 `|` 操作符，但是它不会返回仅匹配右子表达式的文档。

### 否定操作符

```sql
hello -world
hello !world
```

否定操作符强制规则要求某个词不存在。

默认情况下，不支持仅包含否定的查询。要启用此功能，请使用服务器选项 [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed)。

### 字段搜索操作符

```sql
@title hello @body world
```

字段限制操作符将后续搜索限制在指定字段。默认情况下，如果指定的字段名在搜索表中不存在，则查询将失败并返回错误消息。但通过在查询开头指定 `@@relaxed` 选项，可以抑制此行为：

```sql
@@relaxed @nosuchfield my query
```

这在搜索具有不同模式的异构表时非常有用。

字段位置限制进一步将搜索限制在给定字段（或多个字段）的前 N 个位置。例如，`@body [50] hello` 不会匹配关键词 `hello` 出现在正文第 51 位及之后的位置的文档。

```sql
@body[50] hello
```

多字段搜索操作符：

```sql
@(title,body) hello world
```

忽略字段搜索操作符（忽略从 `title` 字段匹配到的“hello world”）：

```sql
@!title hello world
```

忽略多字段搜索操作符（如果有字段 `title`、`subject` 和 `body`，那么 `@!(title)` 等同于 `@(subject,body)`）：

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

短语操作符要求词语必须相邻。

短语搜索操作符可以包含“匹配任何术语”的修饰符。在短语操作符内，词语的位置是重要的。当使用“匹配任何”修饰符时，该短语查询中后续词语的位置将被偏移。因此，“匹配任何”修饰符不影响搜索性能。

```sql
"exact * phrase * * for terms"
```

你还可以在引号内使用 OR 操作符。OR 操作符（`|`）在短语内部使用时，必须用括号`()`包围。每个选项的位置相同，如果任何选项匹配该位置，则短语匹配。

**正确示例**（带括号）：
```sql
"( a | b ) c"
"( ( a b c ) | d ) e"
"man ( happy | sad ) but all ( ( as good ) | ( as fast ) )"
```

**错误示例**（无括号 - 不会生效）：
```sql
"a | b c"
"happy | sad"
```

###  近邻搜索操作符

```sql
"hello world"~10
```

近邻距离以单词数计数，作用于引号内的所有词。如查询 `"cat dog mouse"~5` 表示包含这3个词的跨度必须少于 8 个词。因此，文档 `CAT aaa bbb ccc DOG eee fff MOUSE` 不匹配该查询，因为其跨度正好为 8 个词。

你也可以在近邻搜索中使用 OR 操作符。OR 操作符（`|`）在近邻搜索内使用时，必须用括号`()`包围。每个选项单独检查。

**正确示例**（带括号）：
```sql
"( two | four ) fish chips"~5
```

**错误示例**（无括号 - 无效）：
```sql
"two | four fish chips"~5
```

###  共识匹配操作符

```sql
"the world is a wonderful place"/3
```

共识匹配操作符提供了一种模糊匹配类型。它只匹配达到指定词数阈值的文档。在上述示例中（`"the world is a wonderful place"/3`），它会匹配包含指定6个词中至少3个词的所有文档。该操作符最多支持255个关键词。除指定绝对数量外，你也可以提供0.0至1.0之间的值（表示0%至100%），Manticore只匹配包含给定词中至少指定比例的文档。同样示例也可写作 `"the world is a wonderful place"/0.5`，表示匹配包含6个词中至少50%的文档。

共识操作符支持 OR（`|`）操作符。OR 操作符（`|`）在共识匹配中必须用括号`()`包围。每个 OR 组中只计算一个词用于匹配。

**正确示例**（带括号）：
```sql
"( ( a b c ) | d ) e f g"/0.5
"happy ( sad | angry ) man"/2
```

**错误示例**（无括号 - 无效）：
```sql
"a b c | d e f g"/0.5
```

### 严格顺序操作符

```sql
aaa << bbb << ccc
```

严格顺序操作符（也称为“前置”操作符）仅当查询中指定顺序的关键词按顺序出现在文档中时匹配该文档。例如，查询 `black << cat` 会匹配文档“black and white cat”，但不会匹配文档“that cat was black”。顺序操作符优先级最低。它可应用于单个关键词和更复杂表达式。例如，以下是有效查询：

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### 精确形式修饰符

```sql
raining =cats and =dogs
="exact phrase"
```

精确形式关键词修饰符仅当关键词以指定的精确形式出现时才匹配文档。默认情况下，如果词干化/词形还原后的关键词匹配，即认为文档匹配。例如，查询 “runs” 会匹配包含 “runs” 和 “running” 的文档，因为两者的词干都是 “run”。但是 `=runs` 查询只会匹配包含 “runs” 的文档。精确形式修饰符需要启用 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 选项。

另一个用例是防止将[关键词扩展](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)为其 `*keyword*` 形式。例如，使用 `index_exact_words=1` + `expand_keywords=1/star` 时，`bcd` 会找到包含 `abcde` 的文档，但 `=bcd` 则不会。

作为影响关键词的修饰符，它可以在短语、邻近和法定人数运算符等运算符中使用。可以对短语运算符应用精确形式修饰符，在这种情况下，它会在内部将精确形式修饰符添加到短语中的所有词语。

### 通配符运算符

```sql
nation* *nation* *national
```

前缀（尾部扩展）和/或后缀（头部扩展）需要[最小中缀长度](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。如果仅希望做前缀匹配，则可以使用[最小前缀长度](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)。

搜索将尝试找到所有通配符标记的扩展，每个扩展都会被记录为匹配命中。标记的扩展数量可以通过表设置中的[扩展限制](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit)进行控制。通配符标记可能对查询搜索时间产生显著影响，尤其是当标记长度较短时。在这种情况下，建议使用扩展限制。

如果使用了[expand_keywords](../../Searching/Options.md#expand_keywords)表设置，通配符运算符可以自动应用。

此外，还支持以下内联通配符运算符：

* `?` 可以匹配任意单个字符：`t?st` 将匹配 `test`，但不匹配 `teast`
* `%` 可以匹配零个或一个字符：`tes%` 将匹配 `tes` 或 `test`，但不匹配 `testing`

内联运算符需要启用 `dict=keywords`（默认启用）和开启前缀/中缀匹配。

### REGEX 运算符

```sql
REGEX(/t.?e/)
```

需要设置[最小中缀长度](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)或[最小前缀长度](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)及[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords 选项（这是默认设置）。

类似于[通配符运算符](../../Searching/Full_text_matching/Operators.md#Wildcard-operators)，REGEX 运算符尝试找到所有匹配所提供正则表达式的标记，每个扩展都会被记录为匹配命中。请注意，这可能会显著影响查询搜索时间，因为整个字典都会被扫描，且字典中的每个词条都会与 REGEX 模式进行匹配。

模式应遵循[RE2 语法](https://github.com/google/re2/wiki/Syntax)。REGEX 表达式的定界符是开括号后的第一个符号。换句话说，所有介于开括号后跟着的定界符和该定界符与闭括号之间的文本都被视为 RE2 表达式。
请注意，存储在字典中的词条会经历 `charset_table` 转换，这意味着例如，如果根据 `charset_table`（默认情况）所有字符都转为小写，REGEX 可能无法匹配大写字符。要成功匹配词条，REGEX 模式必须对应整个标记。若要实现部分匹配，请在模式的开头和/或结尾添加 `.*`。

```sql
REGEX(/.{3}t/)
REGEX(/t.*\d*/)
```

### 字段起始与字段结束修饰符

```sql
^hello world$
```

字段起始和字段结束关键词修饰符确保关键词只有在出现在全文字段的最开始或最末尾时才匹配。举例来说，查询 `"^hello world$"`（用引号括起来以将短语运算符与起始/结束修饰符组合）将仅匹配包含至少一个字段里这两个特定关键词的文档。

### IDF 加权修饰符

```sql
boosted^1.234 boostedfieldend$^1.234
```

加权修饰符通过指定的因子提升包含 IDF 计算的排名分数中的词语[逆文档频率](../../Searching/Options.md#idf)_分数。它不会以任何方式影响匹配过程。

### NEAR 运算符

```sql
hello NEAR/3 world NEAR/4 "my test"
```

`NEAR` 运算符是邻近运算符的更通用版本。其语法为 `NEAR/N`，区分大小写，并且不允许在 `NEAR` 关键字、斜杠符号和距离值之间有空格。

原邻近运算符仅适用于关键词集合，而 `NEAR` 更加灵活，可以接受任意子表达式作为两个参数。当文档中两个子表达式在 N 个词以内（顺序不限）时，匹配成立。`NEAR` 是左结合的，并具有与[BEFORE](../../Searching/Full_text_matching/Operators.md#Strict-order-operator)相同的（最低）优先级。

需要注意的是，`one NEAR/7 two NEAR/7 three` 并不完全等同于 `"one two three"~7`。关键区别在于邻近运算符允许这三个匹配词之间最多有6个非匹配词，而使用 `NEAR` 的版本限制较宽松：允许 `one` 和 `two` 之间最多6个词，然后再允许该两个词匹配与 `three` 之间最多6个词。

### NOTNEAR 运算符

```sql
Church NOTNEAR/3 street
```
`NOTNEAR` 运算符作为一个负向断言。当左侧参数存在且右侧参数不存在于文档中，或者右侧参数与左侧匹配参数结尾之间的距离达到指定的词数时，匹配成立。距离以词为单位。语法为 `NOTNEAR/N`，区分大小写，并且不允许在 `NOTNEAR` 关键字、斜杠符号和距离值之间有空格。该运算符的两个参数可以是词语，也可以是任何运算符或运算符组。

### SENTENCE 和 PARAGRAPH 运算符

```sql
all SENTENCE words SENTENCE "in one sentence"
```


```sql
"Bill Gates" PARAGRAPH "Steve Jobs"
```
`SENTENCE` 和 `PARAGRAPH` 操作符在它们的两个参数都位于同一句子或同一段文本时匹配文档。这些参数可以是关键字、短语或相同操作符的实例。

句子或段落中参数的顺序无关紧要。这些操作符仅在启用了[index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)（句子和段落索引功能）的表中有效，否则退化为简单的 AND 操作。有关句子和段落的定义，请参阅[index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)指令文档。


### ZONE 限定操作符

```sql
ZONE:(h3,h4)

only in these titles
```

`ZONE limit` 操作符与字段限定操作符类似，但限制匹配到指定的字段内区域或多个区域列表。值得注意的是，后续的子表达式不需要在给定区域的一个连续范围内匹配，可以跨越多个范围匹配。例如，查询 `(ZONE:th hello world)` 将匹配以下示例文档：

```html
<th>Table 1. Local awareness of Hello Kitty brand.</th>
.. some table data goes here ..
<th>Table 2. World-wide brand awareness.</th>
```

`ZONE` 操作符影响查询，直到下一个字段或 `ZONE` 限定操作符，或者直到闭合括号。它仅对支持区域的表有效（参见[index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)），否则将被忽略。

### ZONESPAN 限定操作符

```sql
ZONESPAN:(h2)

only in a (single) title
```

`ZONESPAN` 限定操作符类似于 `ZONE` 操作符，但要求匹配必须发生在单一连续范围内。在前面给出的示例中，`ZONESPAN:th hello world` 不会匹配该文档，因为“hello”和“world”不在同一个连续范围内。

<!-- proofread -->

