# 完整文本操作符

查询字符串可以包含特定的操作符，用于定义查询字符串中的单词应该如何匹配的条件。

### 布尔操作符

#### AND 操作符

隐式的逻辑 AND 操作符总是存在，因此 "hello world" 意味着匹配文档中必须同时包含 "hello" 和 "world"。

```sql
hello  world
```

注意：没有显式的 `AND` 操作符。

#### OR 操作符

逻辑 OR 操作符 `|` 的优先级高于 AND，因此 `looking for cat | dog | mouse` 意味着 `looking for (cat | dog | mouse)`，而不是 `(looking for cat) | dog | mouse`。

```sql
hello | world
```

注意：没有 `OR` 操作符。请使用 `|`。

### MAYBE 操作符

```sql
hello MAYBE world
```

`MAYBE` 操作符的功能类似于 `|` 操作符，但它不会返回仅匹配右子表达式的文档。

### 否定操作符

```sql
hello -world
hello !world
```

否定操作符强制规则为某个单词不存在。

默认情况下，不支持仅包含否定的查询。要启用此功能，请使用服务器选项 [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed)。

### 字段搜索操作符

```sql
@title hello @body world
```

字段限制操作符将后续搜索限制在指定字段内。默认情况下，如果查询中指定的字段名在搜索表中不存在，查询将失败并返回错误信息。但可以通过在查询开头指定 `@@relaxed` 选项来抑制此行为：

```sql
@@relaxed @nosuchfield my query
```

这在搜索具有不同模式的异构表时非常有用。

字段位置限制进一步约束搜索仅在指定字段的前 N 个位置内进行。例如，`@body [50] hello` 在文档中关键词 `hello` 出现在正文第 51 位或之后时不会匹配。

```sql
@body[50] hello
```

多字段搜索操作符：

```sql
@(title,body) hello world
```

忽略字段搜索操作符（忽略 'title' 字段中 'hello world' 的任何匹配）：

```sql
@!title hello world
```

忽略多字段搜索操作符（如果有字段 'title'、'subject' 和 'body'，则 `@!(title)` 等同于 `@(subject,body)`）：

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

短语操作符要求单词相邻。

短语搜索操作符可以包含“匹配任一词”修饰符。在短语操作符中，词语的位置具有实际意义。当使用“匹配任一词”修饰符时，短语查询中后续词语的位置将被平移。因此，“匹配任一词”修饰符不会影响搜索性能。

注意：当使用此操作符进行包含超过 31 个关键词的查询时，位置在第 31 及以后的关键词的排名统计（如 `tf`、`idf`、`bm25`）可能会被低估。这是因为内部用于跟踪匹配中词项出现的 32 位掩码。匹配逻辑（寻找文档）保持正确，但对于非常长的查询，排名分数可能会受影响。

```sql
"exact * phrase * * for terms"
```

你也可以在引号内使用 OR 操作符。OR 操作符（`|`）在短语中使用时必须用括号 `()` 括起来。对于每个位置，选项依次检查，只要任一选项匹配该位置，短语即匹配。

**正确示例**（带括号）：
```sql
"( a | b ) c"
"( ( a b c ) | d ) e"
"man ( happy | sad ) but all ( ( as good ) | ( as fast ) )"
```

**错误示例**（无括号 - 这些不生效）：
```sql
"a | b c"
"happy | sad"
```

### 临近搜索操作符

```sql
"hello world"~10
```

临近距离以单词数计，包括单词数量，适用于引号内的所有词。例如，查询 `"cat dog mouse"~5` 表示这三个词必须出现在少于 8 个单词的范围内。因此，一个包含 `CAT aaa bbb ccc DOG eee fff MOUSE` 的文档不会匹配这个查询，因为范围正好是 8 个单词。

注意：当使用此操作符进行包含超过 31 个关键词的查询时，位置在第 31 及以后的关键词的排名统计（如 `tf`、`idf`、`bm25`）可能会被低估。这是因为内部用于跟踪匹配中词项出现的 32 位掩码。匹配逻辑（寻找文档）保持正确，但对于非常长的查询，排名分数可能会受影响。

你可以在临近搜索中使用 OR 操作符。OR 操作符（`|`）在临近搜索中使用时必须用括号 `()` 括起来。每个选项分别检查。

**正确示例**（带括号）：
```sql
"( two | four ) fish chips"~5
```

**错误示例**（无括号 - 这不生效）：
```sql
"two | four fish chips"~5
```

### 投票匹配操作符

```sql
"the world is a wonderful place"/3
```

投票匹配操作符引入了一种模糊匹配方式。它只匹配满足指定词语阈值的文档。例如上面的示例 (`"the world is a wonderful place"/3`) 会匹配包含所给 6 个词中至少 3 个词的所有文档。该操作符支持最多 255 个关键词。除了绝对数值外，还可以提供一个介于 0.0 到 1.0 的值（表示 0% 到 100%），Manticore 会匹配至少包含给定词语指定百分比的文档。上述示例也可以写为 `"the world is a wonderful place"/0.5`，将匹配至少包含 6 个词中 50% 的文档。

投票操作符支持 OR (`|`) 操作符。OR 操作符 (`|`) 在投票匹配中使用时必须用括号 `()` 括起来。每个 OR 组中只有一个词计入匹配。

**正确示例**（带括号）：
```sql
"( ( a b c ) | d ) e f g"/0.5
"happy ( sad | angry ) man"/2
```

**错误示例**（无括号 - 这不生效）：
```sql
"a b c | d e f g"/0.5
```

### 严格顺序操作符

```sql
aaa << bbb << ccc
```

严格顺序操作符（也称为“前置”操作符）仅当参数关键字在文档中严格按查询中指定的顺序出现时才匹配该文档。例如，查询 `black << cat` 会匹配文档 "black and white cat" ，但不会匹配 "that cat was black"。顺序操作符优先级最低。它既可以应用于单个关键字，也可应用于更复杂的表达式。例如，这是一条有效查询：

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### 精确形式修饰符

```sql
raining =cats and =dogs
="exact phrase"
```

精确形式关键字修饰符仅当关键字以指定的精确形式出现时才匹配文档。默认情况下，如果词干/词形还原后的关键字匹配，则文档被视为匹配。例如，查询 "runs" 会匹配包含 "runs" 和包含 "running" 的文档，因为这两种形式的词干都是 "run"。但是，`=runs` 查询只会匹配第一个文档。精确形式修饰符需要启用[index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)选项。

另一个用例是防止关键字[扩展](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)为其`*keyword*`形式。例如，在`index_exact_words=1` + `expand_keywords=1/star`的情况下，`bcd` 会找到包含 `abcde` 的文档，但 `=bcd` 不会。

作为影响关键字的修饰符，它可以用于短语、邻近和仲裁操作符中。对短语操作符应用精确形式修饰符是可能的，在这种情况下，它会在内部将精确形式修饰符添加到短语中的所有词项。

### 通配符操作符

```sql
nation* *nation* *national
```

前缀（尾部扩展）和/或后缀（头部扩展）需要 [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。如果只需要前缀，可以使用[min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)。

搜索会尝试找到所有通配符词项的扩展形式，每个扩展都作为匹配命中记录。可以使用[expansion_limit](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit)表设置来控制词项扩展数量。通配符词项对查询搜索时间有显著影响，尤其是词项长度较短时，此时建议使用扩展限制。

如果使用了[expand_keywords](../../Searching/Options.md#expand_keywords)表设置，通配符操作符可以自动应用。

此外，支持以下内联通配符操作符：

* `?` 可以匹配任意单个字符：`t?st` 会匹配 `test`，但不匹配 `teast`
* `%` 可以匹配零个或一个字符：`tes%` 会匹配 `tes` 或 `test`，但不匹配 `testing`

内联操作符需要启用 `dict=keywords`（默认启用）且启用前缀/中缀匹配。

### REGEX 操作符

```sql
REGEX(/t.?e/)
```

需要设置[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)或[min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)和[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords选项（默认）。

与[通配符操作符](../../Searching/Full_text_matching/Operators.md#Wildcard-operators)类似，REGEX操作符尝试找到所有匹配所提供模式的词项，每个扩展都作为匹配命中记录。注意，这可能对查询搜索时间产生显著影响，因为会扫描整个字典，字典中每个词项都要与REGEX模式进行匹配。

模式应遵循[RE2 语法](https://github.com/google/re2/wiki/Syntax)。REGEX表达式的定界符是开括号后的第一个符号。换句话说，开括号后跟定界符与定界符和闭括号之间的所有文本都被视为RE2表达式。  
请注意，存储在字典中的词项会经过 `charset_table` 转换，这意味着例如如果所有字符根据 `charset_table`（默认）被转为小写，REGEX 可能无法匹配大写字符。要成功用 REGEX 表达式匹配词项，模式必须对应整个词项。若要实现部分匹配，请在模式开头和/或结尾添加 `.*`。

```sql
REGEX(/.{3}t/)
REGEX(/t.*\d*/)
```

### 字段起始和字段结束修饰符

```sql
^hello world$
```

字段起始和字段结束关键字修饰符确保关键字仅在其出现在全文字段的开头或结尾时匹配。例如，查询 `"^hello world$"`（用引号括起以结合短语操作符与起始/结束修饰符）将仅匹配包含至少一个字段含有这两个特定关键字的文档。

### IDF 权重提升修饰符

```sql
boosted^1.234 boostedfieldend$^1.234
```

提升修饰符通过指定的因子提升包含 IDF 计算的排名分数中的词汇[IDF](../../Searching/Options.md#idf)_得分。但它不会以任何方式影响匹配过程。

### NEAR 操作符

```sql
hello NEAR/3 world NEAR/4 "my test"
```

`NEAR` 操作符是邻近操作符的更通用版本。其语法为 `NEAR/N`，区分大小写，且 `NEAR` 关键字、斜杠符号和距离值之间不允许有空格。

原始邻近操作符仅对关键字集合起作用，而 `NEAR` 更灵活，可以接受任意子表达式作为两个参数。当两个子表达式相距不超过 N 个词时，无论顺序如何，都匹配文档。`NEAR` 为左结合，其优先级与[BEFORE](../../Searching/Full_text_matching/Operators.md#Strict-order-operator)相同且都是最低优先级。

需要注意的是，`one NEAR/7 two NEAR/7 three` 与 `"one two three"~7` 并不完全等价。关键区别在于邻近操作符允许所有三个匹配词之间最多有 6 个不匹配词，而使用 `NEAR` 的版本限制较松：它允许 `one` 和 `two` 之间最多有 6 个词，然后再允许该两个词匹配与 `three` 之间最多有另外 6 个词。

注意：当使用此运算符处理包含超过31个关键词的查询时，位置31及以上的关键词的排名统计信息（如`tf`、`idf`、`bm25`）可能会被低估。这是由于内部使用32位掩码来跟踪匹配项内的词项出现情况。匹配逻辑（查找文档）保持正确，但对于非常长的查询，排名分数可能会受到影响。

### NOTNEAR 运算符

```sql
Church NOTNEAR/3 street
```

`NOTNEAR` 运算符作为否定断言，是 `NEAR` 运算符的逻辑逆运算。当左参数存在，且右参数要么在文档中不存在，要么位于距离左参数**超过**指定距离的位置时，它匹配文档。

语法为 `NOTNEAR/N`，区分大小写，并且不允许在 `NOTNEAR` 关键字、斜杠符号和距离值之间存在空格。

关键行为包括：
*   **对称性**：与 `NEAR` 类似，`NOTNEAR` 运算符的适用性与文本中词项的顺序无关。如果右参数在指定距离内（无论是左参数**之前**还是**之后**）被找到，匹配将被排除。
*   **距离阈值**：距离 `N` 表示邻近范围（包含）。如果词项之间相隔 `N` 个或更少的词，则匹配被丢弃。右参数必须距离左参数 `N + 1` 个或更多的词。
*   **参数**：此运算符的两个参数可以是词项、短语或运算符组。

### SENTENCE 和 PARAGRAPH 运算符

```sql
all SENTENCE words SENTENCE "in one sentence"
```


```sql
"Bill Gates" PARAGRAPH "Steve Jobs"
```
`SENTENCE` 和 `PARAGRAPH` 运算符分别在它们的两个参数位于同一句子或同一段落内时匹配文档。这些参数可以是关键词、短语或同一运算符的实例。

参数在句子或段落内的顺序无关紧要。这些运算符仅在使用 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)（句子和段落索引功能）构建的表中生效，否则将退化为简单的 AND 操作。关于什么构成句子和段落的定义，请参阅 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp) 指令文档。


### ZONE 限制运算符

```sql
ZONE:(h3,h4)

only in these titles
```

`ZONE limit` 运算符与字段限制运算符非常相似，但将匹配限制在指定的字段内区域或区域列表中。需要注意的是，后续的子表达式不需要在给定区域的单个连续跨度内匹配，它们可以跨多个跨度匹配。例如，查询 `(ZONE:th hello world)` 将匹配以下示例文档：

```html
<th>Table 1. Local awareness of Hello Kitty brand.</th>
.. some table data goes here ..
<th>Table 2. World-wide brand awareness.</th>
```

`ZONE` 运算符会影响查询，直到遇到下一个字段或 `ZONE` 限制运算符，或者直到遇到右括号。它仅在使用区域支持构建的表（参见 [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)）中生效，否则将被忽略。

### ZONESPAN 限制运算符

```sql
ZONESPAN:(h2)

only in a (single) title
```

`ZONESPAN` 限制运算符类似于 `ZONE` 运算符，但要求匹配发生在单个连续跨度内。在前面提供的示例中，`ZONESPAN:th hello world` 将不会匹配该文档，因为 "hello" 和 "world" 没有出现在同一个跨度内。

<!-- proofread -->

