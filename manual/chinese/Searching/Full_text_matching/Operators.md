# 全文检索运算符

查询字符串可以包含特定的运算符，这些运算符定义了如何匹配查询字符串中的单词的条件。

### 布尔运算符

#### AND 运算符

隐式的逻辑 AND 运算符始终存在，因此 "hello world" 意味着必须在匹配的文档中同时找到 "hello" 和 "world"。

```sql
hello  world
```

注意：没有显式的 `AND` 运算符。

#### OR 运算符

逻辑 OR 运算符 `|` 的优先级高于 AND，因此 `looking for cat | dog | mouse` 表示 `looking for (cat | dog | mouse)` 而不是 `(looking for cat) | dog | mouse`。

```sql
hello | world
```

注意：没有 `OR` 运算符。请改用 `|`。

### MAYBE 运算符

```sql
hello MAYBE world
```

`MAYBE` 运算符的功能类似于 `|` 运算符，但它不会返回仅匹配右子树表达式的文档。

### 否定运算符

```sql
hello -world
hello !world
```

否定运算符强制执行单词不存在的规则。

默认情况下，**仅**包含否定的查询是**不**支持的。要启用，请使用服务器选项 [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed)。

### 字段搜索运算符

```sql
@title hello @body world
```

字段限制运算符将后续搜索限制在指定的字段。默认情况下，如果给定的字段名在搜索的表中不存在，查询将失败并显示错误消息。但是，可以通过在查询开头指定 `@@relaxed` 选项来抑制此行为：

```sql
@@relaxed @nosuchfield my query
```

这在搜索具有不同架构的异构表时很有用。

字段位置限制还可以将搜索进一步限制在给定字段内的前 N 个位置。例如，`@body [50] hello` 将不会匹配关键字 `hello` 出现在正文第 51 位或之后的文档。

```sql
@body[50] hello
```

多字段搜索运算符：

```sql
@(title,body) hello world
```

忽略字段搜索运算符（忽略 'title' 字段中 'hello world' 的任何匹配）：

```sql
@!title hello world
```

忽略多字段搜索运算符（如果有 'title'、'subject' 和 'body' 字段，则 `@!(title)` 等同于 `@(subject,body)`）：

```sql
@!(title,body) hello world
```

全字段搜索运算符：

```sql
@* hello
```

### 短语搜索运算符

```sql
"hello world"
```

短语运算符要求单词彼此相邻。

短语搜索运算符可以包含 `匹配任意词` 修饰符。在短语运算符内，词的位置很重要。当使用 '匹配任意' 修饰符时，后续词在该短语查询中的位置将被移动。因此，'匹配任意' 修饰符不会影响搜索性能。

注意：当使用包含超过 31 个关键词的查询时，位置 31 及以上的关键词的排名统计（如 `tf`、`idf`、`bm25`）可能会被低估。这是因为内部使用了 32 位掩码来跟踪匹配中的词出现情况。匹配逻辑（查找文档）仍然是正确的，但对于非常长的查询，排名分数可能会受到影响。

```sql
"exact * phrase * * for terms"
```

你还可以在引号内使用 OR 运算符。在短语内使用时，OR 运算符 (`|`) 必须用括号 `()` 括起来。每个选项在相同位置被检查，如果任何选项匹配该位置，则短语匹配。

**正确示例**（使用括号）：
```sql
"( a | b ) c"
"( ( a b c ) | d ) e"
"man ( happy | sad ) but all ( ( as good ) | ( as fast ) )"
```

**错误示例**（没有括号 - 这些不会起作用）：
```sql
"a | b c"
"happy | sad"
```

### 邻近搜索运算符

```sql
"hello world"~10
```

邻近距离以单词计算，考虑单词数，并适用于引号内的所有单词。例如，查询 `"cat dog mouse"~5` 表示必须有少于 8 个单词的跨度包含所有 3 个单词。因此，包含 `CAT aaa bbb ccc DOG eee fff MOUSE` 的文档将不匹配此查询，因为跨度恰好是 8 个单词长。

注意：当使用包含超过 31 个关键词的查询时，位置 31 及以上的关键词的排名统计（如 `tf`、`idf`、`bm25`）可能会被低估。这是因为内部使用了 32 位掩码来跟踪匹配中的词出现情况。匹配逻辑（查找文档）仍然是正确的，但对于非常长的查询，排名分数可能会受到影响。

你还可以在邻近搜索中使用 OR 运算符。在邻近搜索中使用时，OR 运算符 (`|`) 必须用括号 `()` 括起来。每个选项单独检查。

**正确示例**（使用括号）：
```sql
"( two | four ) fish chips"~5
```

**错误示例**（没有括号 - 这不会起作用）：
```sql
"two | four fish chips"~5
```

### 法定人数匹配运算符

```sql
"the world is a wonderful place"/3
```

法定人数匹配运算符引入了一种模糊匹配。它只匹配满足指定单词给定阈值的文档。在上面的示例中（`"the world is a wonderful place"/3`），它将匹配包含至少 6 个指定单词中的 3 个的所有文档。运算符限制为 255 个关键词。除了绝对数字外，你还可以提供 0.0 到 1.0 之间的值（分别代表 0% 和 100%），Manticore 将仅匹配包含给定词至少指定百分比的文档。上面的同一示例也可以表示为 `"the world is a wonderful place"/0.5`，它将匹配包含 6 个词中至少 50% 的文档。

法定人数运算符支持 OR (`|`) 运算符。在法定人数匹配中使用时，OR 运算符 (`|`) 必须用括号 `()` 括起来。只有每个 OR 组中的一个词计入匹配。

**正确示例**（使用括号）：
```sql
"( ( a b c ) | d ) e f g"/0.5
"happy ( sad | angry ) man"/2
```

**错误示例**（没有括号 - 这不会起作用）：
```sql
"a b c | d e f g"/0.5
```

### 严格顺序运算符

```sql
aaa << bbb << ccc
```

严格顺序运算符（也称为"之前"运算符）只有在其参数关键词以查询中指定的确切顺序出现在文档中时，才匹配一个文档。例如，查询 `black << cat` 将匹配文档 "black and white cat"，但不匹配文档 "that cat was black"。顺序运算符具有最低的优先级。它可以应用于单个关键词和更复杂的表达式。例如，这是一个有效的查询：

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### 精确形式修饰符

```sql
raining =cats and =dogs
="exact phrase"
```

精确形式关键词修饰符仅在关键词以指定的确切形式出现时才匹配文档。默认情况下，如果关键词的词干/词形还原匹配，则认为文档是匹配的。例如，查询"runs"将同时匹配包含"runs"和"running"的文档，因为两者的词干都是"run"。但是，`=runs`查询将仅匹配第一个文档。精确形式修饰符要求启用 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 选项。

另一个用例是防止将关键词扩展为 `*keyword*` 形式。例如，使用 `index_exact_words=1` + `expand_keywords=1/star`，`bcd` 将查找包含 `abcde` 的文档，但 `=bcd` 不会。

作为影响关键词的修饰符，它可以在短语、邻近性和仲裁运算符等运算符中使用。可以对短语运算符应用精确形式修饰符，在这种情况下，它会在内部为短语中的所有项添加精确形式修饰符。

### 通配符运算符

```sql
nation* *nation* *national
```

需要 [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 来进行前缀（尾部扩展）和/或后缀（头部扩展）。如果只需要前缀，可以使用 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)。

搜索将尝试查找所有通配符标记的扩展，并将每个扩展记录为匹配命中。可以使用 [expansion_limit](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) 表设置控制标记的扩展次数。通配符标记可能对查询搜索时间有显著影响，尤其是当标记长度较短时。在这种情况下，使用扩展限制是可取的。

如果使用 [expand_keywords](../../Searching/Options.md#expand_keywords) 表设置，通配符运算符可以自动应用。

此外，还支持以下内联通配符运算符：

* `?` 可以匹配任何单个字符：`t?st` 将匹配 `test`，但不匹配 `teast`
* `%` 可以匹配零个或一个字符：`tes%` 将匹配 `tes` 或 `test`，但不匹配 `testing`

内联运算符需要 `dict=keywords`（默认启用）和启用前缀/中缀。

### REGEX运算符

```sql
REGEX(/t.?e/)
```

需要设置 [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 或 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) 和 [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords 选项（这是默认设置）。

与[通配符运算符](../../Searching/Full_text_matching/Operators.md#Wildcard-operators)类似，REGEX运算符尝试查找所有匹配提供的模式的标记，并将每个扩展记录为匹配命中。请注意，这可能对查询搜索时间产生显著影响，因为需要扫描整个词典，并且词典中的每个术语都要与REGEX模式匹配。

模式应遵循 [RE2 语法](https://github.com/google/re2/wiki/Syntax)。REGEX表达式分隔符是开括号后的第一个符号。换句话说，开括号后的分隔符和分隔符与闭括号之间的所有文本都被视为RE2表达式。
请注意，存储在词典中的术语会经过 `charset_table` 转换，这意味着例如，如果根据 `charset_table`（默认情况下）将所有字符转换为小写，则REGEX可能无法匹配大写字符。要使用REGEX表达式成功匹配术语，模式必须对应整个标记。要实现部分匹配，请在模式的开头和/或结尾放置 `.*`。

```sql
REGEX(/.{3}t/)
REGEX(/t.*\d*/)
```

### 字段起始和字段结束修饰符

```sql
^hello world$
```

字段起始和字段结束关键词修饰符确保关键词仅在全文字段的开头或结尾处匹配。例如，查询 `"^hello world$"`（用引号括起来以将短语运算符与起始/结束修饰符结合）将只匹配包含至少一个具有这两个特定关键词的字段的文档。

### IDF提升修饰符

```sql
boosted^1.234 boostedfieldend$^1.234
```

提升修饰符通过指定的因子提高词语的 [IDF](../../Searching/Options.md#idf) 分数，以纳入 IDF 的排名分数计算中。它不会以任何方式影响匹配过程。

### NEAR运算符

```sql
hello NEAR/3 world NEAR/4 "my test"
```

`NEAR` 运算符是邻近运算符的更通用版本。其语法是 `NEAR/N`，区分大小写，并且 `NEAR` 关键词、斜杠符号和距离值之间不允许有空格。

虽然原始邻近运算符仅适用于关键词集合，但 `NEAR` 更加通用，可以接受任意子表达式作为其两个参数。当两个子表达式在 N 个词以内找到时，无论其顺序如何，它都匹配一个文档。`NEAR` 是左结合的，并且与 [BEFORE](../../Searching/Full_text_matching/Operators.md#Strict-order-operator) 共享相同的（最低）优先级。

需要注意的是，`one NEAR/7 two NEAR/7 three` 不完全等同于 `"one two three"~7`。关键区别在于，邻近运算符允许在所有三个匹配词之间最多有6个非匹配词，而带有 `NEAR` 的版本限制较少：它允许 `one` 和 `two` 之间最多6个词，然后在该两词匹配和 `three` 之间再允许最多6个词。

注意：在使用包含超过31个关键词的查询时，排名统计（如 `tf`、`idf`、`bm25`）可能会对位置在31及以上的关键词进行计数不足。这是由于内部使用了32位掩码来跟踪匹配中的术语出现次数。匹配逻辑（查找文档）仍然正确，但对于非常长的查询，排名得分可能会受到影响。

### NOTNEAR 操作符

```sql
Church NOTNEAR/3 street
```

`NOTNEAR` 操作符作为一个否定断言，功能上相当于 `NEAR` 操作符的逻辑反面。当左侧参数出现时，若右侧参数要么在文档中不存在，要么距离左侧参数**超过**指定距离，则匹配该文档。

语法为 `NOTNEAR/N`，区分大小写，并且 `NOTNEAR` 关键词、斜杠和距离值之间不允许有空格。

主要行为包括：
*   **对称性**：和 `NEAR` 一样，`NOTNEAR` 操作符无视文本中术语的顺序。如果右侧参数出现在指定距离内，无论是在左侧参数之前还是之后，匹配都会被排除。
*   **距离阈值**：距离 `N` 表示附近范围（含）。如果两个词之间相隔 `N` 个词或更少，该匹配被丢弃。右侧参数必须距离左侧参数 `N + 1` 个词或更多。
*   **参数**：该操作符的两个参数可以是术语、短语或操作符组。

### SENTENCE 和 PARAGRAPH 操作符

```sql
all SENTENCE words SENTENCE "in one sentence"
```


```sql
"Bill Gates" PARAGRAPH "Steve Jobs"
```
当两个参数都位于同一句子或同一段落内时，`SENTENCE` 和 `PARAGRAPH` 操作符分别匹配文档。这些参数可以是关键词、短语或相同操作符的实例。

句子或段落内参数的顺序无关紧要。这些操作符仅在启用了 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)（句子和段落索引功能）的表中生效，否则退化为简单的 AND 操作。关于句子和段落的定义，请参阅 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp) 指令文档。


### ZONE 限制操作符

```sql
ZONE:(h3,h4)

only in these titles
```

`ZONE limit` 操作符与字段限制操作符非常相似，但将匹配限制在指定的字段区段或区段列表中。需要注意的是，后续子表达式不必在给定区段的单个连续范围内匹配，也可以跨多个范围匹配。例如，查询 `(ZONE:th hello world)` 将匹配以下示例文档：

```html
<th>Table 1. Local awareness of Hello Kitty brand.</th>
.. some table data goes here ..
<th>Table 2. World-wide brand awareness.</th>
```

`ZONE` 操作符对查询的作用范围持续至下一个字段或 `ZONE` 限制操作符，或直至闭合括号。它仅在启用了区段支持的表中生效（参见 [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)），否则会被忽略。

### ZONESPAN 限制操作符

```sql
ZONESPAN:(h2)

only in a (single) title
```

`ZONESPAN` 限制操作符类似于 `ZONE` 操作符，但要求匹配发生在单个连续范围内。在前述示例中，`ZONESPAN:th hello world` 将不会匹配该文档，因为“hello”和“world”不在同一范围内。

<!-- proofread -->

