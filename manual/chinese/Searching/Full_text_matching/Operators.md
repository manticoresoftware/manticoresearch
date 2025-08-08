# 完全文本操作符

查询字符串可以包含特定的操作符，用于定义查询字符串中的词如何被匹配的条件。

### 布尔操作符

#### AND 操作符

隐含的逻辑 AND 操作符总是存在，因此 "hello world" 意味着匹配文档中必须同时包含 "hello" 和 "world"。

```sql
hello  world
```

注意：没有显式的 `AND` 操作符。

#### OR 操作符

逻辑 OR 操作符 `|` 的优先级高于 AND，所以 `looking for cat | dog | mouse` 表示 `looking for (cat | dog | mouse)` 而不是 `(looking for cat) | dog | mouse`。

```sql
hello | world
```

注意：没有操作符 `OR`。请使用 `|`。

### MAYBE 操作符

```sql
hello MAYBE world
```

`MAYBE` 操作符的作用类似于 `|` 操作符，但不会返回仅匹配右侧子表达式的文档。

### 否定操作符

```sql
hello -world
hello !world
```

否定操作符规定某个词不得出现。

默认情况下，不支持仅包含否定的查询。欲启用此功能，请使用服务器选项 [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed)。

### 字段搜索操作符

```sql
@title hello @body world
```

字段限制操作符将后续搜索限定在指定字段中。默认情况下，如果指定字段不存在于搜索表中，查询将失败并显示错误信息。但通过在查询开头添加 `@@relaxed` 选项，可以抑制该行为：

```sql
@@relaxed @nosuchfield my query
```

这在搜索具有不同模式的异构表时很有用。

字段位置限制进一步将搜索限定在指定字段（或字段集）的前 N 个位置。例如，`@body [50] hello` 不会匹配关键词 `hello` 出现在正文中第 51 位置及之后的文档。

```sql
@body[50] hello
```

多字段搜索操作符：

```sql
@(title,body) hello world
```

忽略字段搜索操作符（忽略从 'title' 字段中匹配到的 'hello world'）：

```sql
@!title hello world
```

忽略多字段搜索操作符（如果字段有 'title'、'subject' 和 'body'，则 `@!(title)` 等同于 `@(subject,body)`）：

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

短语操作符要求词必须相邻。

短语搜索操作符可以包含“匹配任意词”修饰符。在短语操作符内，词的位置很重要。当使用“匹配任意词”修饰符时，短语查询中后续词的位置会被偏移。因此，“匹配任意词”修饰符不会影响搜索性能。

```sql
"exact * phrase * * for terms"
```

你也可以在引号内使用 `OR` 操作符。每个选项在同一位置被检查，如果任何选项匹配该位置，则短语匹配。例如：

```sql
"( a | b ) c"
"( ( a b c ) | d ) e"
"man ( happy | sad ) but all ( ( as good ) | ( as fast ) )"
```

### 近似搜索操作符

```sql
"hello world"~10
```

近似距离以词数计量，适用于引号内的所有词。例如，查询 `"cat dog mouse"~5` 表示必须在少于 8 词的间隔内包含所有 3 个词。因此，包含 `CAT aaa bbb ccc DOG eee fff MOUSE` 的文档不会匹配此查询，因为间隔正好是 8 词。

你也可以在近似搜索中使用 `OR`。每个选项分别检查。例如：

```sql
"( two | four ) fish chips"~5
```

### 召回匹配操作符

```sql
"the world is a wonderful place"/3
```

召回匹配操作符引入一种模糊匹配。它仅匹配满足特定词数阈值的文档。上述例子 (`"the world is a wonderful place"/3`) 会匹配包含指定的 6 个词中至少 3 个的所有文档。该操作符限制为最多 255 个关键字。除了绝对数值外，还可以提供介于 0.0 和 1.0 之间的值（表示 0% 到 100%），Manticore 会匹配至少包含指定百分比词数的文档。上面的例子也可写为 `"the world is a wonderful place"/0.5`，表示匹配至少含有 6 个词中 50% 的文档。

召回操作符支持 `OR` 操作符。每个 `OR` 组内仅计入一个词的匹配。例如：

```sql
"( ( a b c ) | d ) e f g"/0.5
"happy ( sad | angry ) man"/2
```

### 严格顺序操作符

```sql
aaa << bbb << ccc
```

严格顺序操作符（又称“之前”操作符）仅在查询中指定的关键词严格按照顺序出现在文档中时匹配。例如，查询 `black << cat` 会匹配文档 "black and white cat"，但不会匹配文档 "that cat was black"。顺序操作符优先级最低。它既可应用于单个关键词，也可应用于复杂表达式。例如，以下查询是有效的：

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### 精确形态修饰符

```sql
raining =cats and =dogs
="exact phrase"
```

精确形式关键字修饰符仅当关键字以指定的精确形式出现时，才匹配文档。默认情况下，如果词干化/词形还原后的关键字匹配，则文档被视为匹配。例如，查询“runs”将匹配包含“runs”和“running”的文档，因为这两个形式词干均为“run”。然而，`=runs`查询只会匹配第一个文档。精确形式修饰符需要启用[index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)选项。

另一个用例是防止将关键字[扩展](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)为其`*keyword*`形式。例如，结合使用`index_exact_words=1` + `expand_keywords=1/star`时，`bcd`会找到包含`abcde`的文档，但`=bcd`则不会。

作为影响关键字的修饰符，它可以用于短语、邻近和法定人数等操作符中。对短语操作符应用精确形式修饰符是可能的，在这种情况下，它会内部为短语中的所有词项添加精确形式修饰符。

### 通配符操作符

```sql
nation* *nation* *national
```

需要为前缀（尾部扩展）和/或后缀（头部扩展）设置[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。如果只需前缀匹配，则可以使用[min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)。

搜索将尝试找到通配符词项的所有扩展，每个扩展记录为匹配命中。词项的扩展数量可以通过[expansion_limit](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit)表设置控制。通配符词项对查询搜索时间有显著影响，特别是当词项长度较短时，此时建议使用扩展限制。

如果使用了[expand_keywords](../../Searching/Options.md#expand_keywords)表设置，通配符操作符可以自动应用。

另外，支持以下内联通配符操作符：

* `?` 可以匹配任意单个字符：`t?st`将匹配`test`，但不匹配`teast`
* `%` 可以匹配零个或一个字符：`tes%`将匹配`tes`或`test`，但不匹配`testing`

内联操作符需要启用`dict=keywords`（默认启用）以及前缀和中缀匹配。

### 正则表达式操作符

```sql
REGEX(/t.?e/)
```

需要设置[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)或[min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)和[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords选项（默认设置）。

类似于[通配符操作符](../../Searching/Full_text_matching/Operators.md#Wildcard-operators)，REGEX操作符尝试找到与提供的模式匹配的所有词项，每个扩展均记录为匹配命中。注意，这会极大影响查询搜索时间，因为要扫描整个字典，字典中的每个词条都要与REGEX模式匹配。

模式应遵循[RE2语法](https://github.com/google/re2/wiki/Syntax)。REGEX表达式的定界符是开括号后的第一个符号。换言之，开括号后跟定界符与定界符和闭括号之间的所有文本被视为RE2表达式。
请注意，存储在字典中的词项会经过`charset_table`转换，这意味着例如，如果所有字符均根据`charset_table`（默认情况）小写化，则REGEX可能无法匹配大写字符。要成功使用REGEX表达式匹配词项，模式必须对应整个词项。若要实现部分匹配，请在模式开头和/或结尾放置`.*`。

```sql
REGEX(/.{3}t/)
REGEX(/t.*\d*/)
```

### 字段开始和字段结束修饰符

```sql
^hello world$
```

字段开始和字段结束关键字修饰符确保仅当关键字出现在全文字段的最开始或最末尾时匹配。例如，查询`"^hello world$"`（用引号括起来以将短语操作符与开始/结束修饰符结合）只会匹配包含至少一个字段具有这两个特定关键词的文档。

### IDF提升修饰符

```sql
boosted^1.234 boostedfieldend$^1.234
```

提升修饰符通过指示的因子提升包含IDF计算的排名分数中的单词[IDF](../../Searching/Options.md#idf)_得分。它不影响匹配过程。

### NEAR操作符

```sql
hello NEAR/3 world NEAR/4 "my test"
```

`NEAR`操作符是邻近操作符的更通用版本。其语法为`NEAR/N`，大小写敏感，且`NEAR`关键字、斜杠和距离值之间不允许有空格。

原始邻近操作符仅适用于关键字集合，而`NEAR`更灵活，可接受任意子表达式作为两个参数。当两个子表达式在N个词内出现时，无论顺序如何，文档即匹配。`NEAR`为左结合，并且与[BEFORE](../../Searching/Full_text_matching/Operators.md#Strict-order-operator)操作符共用最低优先级。
需要注意的是，`one NEAR/7 two NEAR/7 three` 并不完全等同于 `"one two three"~7`。关键区别在于邻近操作符允许在所有三个匹配词之间最多有 6 个不匹配的词，而带有 `NEAR` 的版本限制较少：它允许 `one` 和 `two` 之间最多有 6 个词，然后在该两个词匹配与 `three` 之间再允许最多 6 个词。

### NOTNEAR 操作符

```sql
Church NOTNEAR/3 street
```
`NOTNEAR` 操作符作为负向断言使用。当左侧参数存在且右侧参数要么不存在于文档中，要么与左侧匹配参数的结尾相距指定距离时，该操作符匹配文档。距离以词语数表示。语法是 `NOTNEAR/N`，区分大小写，且 `NOTNEAR` 关键字、斜线和距离值之间不允许有空格。此操作符的两个参数都可以是词条或任何操作符或操作符群组。

### SENTENCE 和 PARAGRAPH 操作符

```sql
all SENTENCE words SENTENCE "in one sentence"
```


```sql
"Bill Gates" PARAGRAPH "Steve Jobs"
```
`SENTENCE` 和 `PARAGRAPH` 操作符用于匹配当它们的两个参数均处于同一文本句子中或同一段落中时的文档。这些参数可以是关键字、短语或同一操作符的实例。

参数在句子或段落中的顺序无关紧要。这些操作符仅在启用[索引_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)（句子和段落索引功能）的表中有效，否则退化为简单的 AND 操作。有关句子和段落定义的信息，请参阅[index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)指令文档。


### ZONE 限制操作符

```sql
ZONE:(h3,h4)

only in these titles
```

`ZONE 限制`操作符与字段限制操作符非常相似，但限制匹配到指定字段内的区域或区域列表。需要注意的是，后续子表达式不需要在给定区域的单个连续区间内匹配，可以跨多个区间匹配。例如，查询 `(ZONE:th hello world)` 将匹配以下示例文档：

```html
<th>Table 1. Local awareness of Hello Kitty brand.</th>
.. some table data goes here ..
<th>Table 2. World-wide brand awareness.</th>
```

`ZONE` 操作符影响查询，直到遇到下一个字段或 `ZONE` 限制操作符或右括号为止。它仅在启用区域支持的表中有效（参见[index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)），否则会被忽略。

### ZONESPAN 限制操作符

```sql
ZONESPAN:(h2)

only in a (single) title
```

`ZONESPAN` 限制操作符与 `ZONE` 操作符类似，但要求匹配必须发生在单个连续区间内。在前述示例中，`ZONESPAN:th hello world` 将不会匹配该文档，因为“hello”与“world”不在同一区间内。

<!-- proofread -->


