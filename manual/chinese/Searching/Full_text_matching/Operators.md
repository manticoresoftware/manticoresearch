# 全文操作符

查询字符串可以包含特定的操作符，用于定义查询字符串中的单词应如何匹配的条件。

### 布尔操作符

#### AND 操作符

隐式的逻辑 AND 操作符始终存在，因此 "hello world" 意味着匹配的文档中必须同时包含 "hello" 和 "world"。

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

`MAYBE` 操作符的功能类似于 `|` 操作符，但它不会返回仅匹配右子树表达式的文档。

### 否定操作符

```sql
hello -world
hello !world
```

否定操作符强制规则要求某个单词不存在。

默认情况下，不支持仅包含否定的查询。要启用此功能，请使用服务器选项 [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed)。

### 字段搜索操作符

```sql
@title hello @body world
```

字段限制操作符将后续搜索限制在指定字段。默认情况下，如果给定的字段名在被搜索的表中不存在，查询将失败并显示错误信息。但是，可以通过在查询开头指定 `@@relaxed` 选项来抑制此行为：

```sql
@@relaxed @nosuchfield my query
```

这在搜索具有不同模式的异构表时非常有用。

字段位置限制进一步将搜索限制在给定字段（或字段组）的前 N 个位置。例如，`@body [50] hello` 不会匹配关键词 `hello` 出现在正文中第 51 位或之后的文档。

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

忽略多字段搜索操作符（如果存在字段 'title'、'subject' 和 'body'，则 `@!(title)` 等同于 `@(subject,body)`）：

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

短语操作符要求单词必须相邻。

短语搜索操作符可以包含“匹配任一词”修饰符。在短语操作符内，词语的位置是有意义的。当使用“匹配任一词”修饰符时，该短语查询中后续词语的位置将被调整。因此，“匹配任一词”修饰符不会影响搜索性能。

```sql
"exact * phrase * * for terms"
```

你也可以在引号内使用 OR 操作符。OR 操作符 (`|`) 在短语内使用时必须用括号 `()` 括起来。每个选项都会在相同位置进行检查，如果任一选项匹配该位置，则短语匹配。

**正确示例**（带括号）：
```sql
"( a | b ) c"
"( ( a b c ) | d ) e"
"man ( happy | sad ) but all ( ( as good ) | ( as fast ) )"
```

**错误示例**（无括号 - 这些不会生效）：
```sql
"a | b c"
"happy | sad"
```

###  近邻搜索操作符

```sql
"hello world"~10
```

近邻距离以单词数计量，适用于引号内的所有单词。例如，查询 `"cat dog mouse"~5` 表示必须有一个少于 8 个单词的范围包含这 3 个单词。因此，包含 `CAT aaa bbb ccc DOG eee fff MOUSE` 的文档不会匹配此查询，因为该范围正好是 8 个单词长。

你也可以在近邻搜索中使用 OR 操作符。OR 操作符 (`|`) 在近邻搜索内使用时必须用括号 `()` 括起来。每个选项单独检查。

**正确示例**（带括号）：
```sql
"( two | four ) fish chips"~5
```

**错误示例**（无括号 - 这不会生效）：
```sql
"two | four fish chips"~5
```

###  法定人数匹配操作符

```sql
"the world is a wonderful place"/3
```

法定人数匹配操作符引入了一种模糊匹配类型。它只匹配满足指定单词阈值的文档。在上例（`"the world is a wonderful place"/3`）中，它将匹配包含至少 3 个指定单词的所有文档。该操作符限制最多 255 个关键词。你也可以提供一个介于 0.0 和 1.0 之间的值（表示 0% 到 100%），Manticore 将只匹配包含至少指定百分比给定单词的文档。上述示例也可以写成 `"the world is a wonderful place"/0.5`，它将匹配包含至少 6 个单词中 50% 的文档。

法定人数操作符支持 OR (`|`) 操作符。OR 操作符 (`|`) 在法定人数匹配中使用时必须用括号 `()` 括起来。每个 OR 组中只有一个单词计入匹配。

**正确示例**（带括号）：
```sql
"( ( a b c ) | d ) e f g"/0.5
"happy ( sad | angry ) man"/2
```

**错误示例**（无括号 - 这不会生效）：
```sql
"a b c | d e f g"/0.5
```

### 严格顺序操作符

```sql
aaa << bbb << ccc
```

严格顺序操作符（也称为“之前”操作符）仅当其参数关键词在文档中严格按照查询中指定的顺序出现时才匹配文档。例如，查询 `black << cat` 会匹配文档 "black and white cat"，但不会匹配文档 "that cat was black"。顺序操作符优先级最低。它可以应用于单个关键词和更复杂的表达式。例如，以下是一个有效查询：

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### 精确形式修饰符

```sql
raining =cats and =dogs
="exact phrase"
```

精确形式关键词修饰符仅当关键词以指定的精确形式出现时才匹配文档。默认情况下，如果词干/词形还原后的关键词匹配，则文档被视为匹配。例如，查询 "runs" 会匹配包含 "runs" 和包含 "running" 的文档，因为两者词干均为 "run"。但是，`=runs` 查询只会匹配第一个文档。精确形式修饰符需要启用 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 选项。

另一个用例是防止将[扩展](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)关键字为其`*keyword*`形式。例如，使用`index_exact_words=1` + `expand_keywords=1/star`时，`bcd`将找到包含`abcde`的文档，但`=bcd`则不会。

作为影响关键字的修饰符，它可以在短语、邻近和法定人数操作符等操作符中使用。可以将精确形式修饰符应用于短语操作符，在这种情况下，它会在内部将精确形式修饰符添加到短语中的所有词项。

### 通配符操作符

```sql
nation* *nation* *national
```

需要为前缀（尾部扩展）和/或后缀（头部扩展）设置[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。如果只需要前缀，可以改用[min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)。

搜索将尝试找到通配符词项的所有扩展，每个扩展都被记录为匹配命中。词项的扩展数量可以通过[expansion_limit](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit)表设置进行控制。通配符词项可能对查询搜索时间产生显著影响，尤其是当词项长度较短时。在这种情况下，建议使用扩展限制。

如果使用了[expand_keywords](../../Searching/Options.md#expand_keywords)表设置，通配符操作符可以自动应用。

此外，支持以下内联通配符操作符：

* `?` 可以匹配任意单个字符：`t?st`将匹配`test`，但不匹配`teast`
* `%` 可以匹配零个或一个字符：`tes%`将匹配`tes`或`test`，但不匹配`testing`

内联操作符需要启用`dict=keywords`（默认启用）以及前缀/中缀启用。

### REGEX操作符

```sql
REGEX(/t.?e/)
```

需要设置[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)或[min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)和[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords选项（默认设置）。

类似于[通配符操作符](../../Searching/Full_text_matching/Operators.md#Wildcard-operators)，REGEX操作符尝试找到所有匹配提供模式的词项，每个扩展都被记录为匹配命中。请注意，这可能对查询搜索时间产生显著影响，因为会扫描整个字典，字典中的每个词项都要与REGEX模式进行匹配。

模式应遵循[RE2语法](https://github.com/google/re2/wiki/Syntax)。REGEX表达式的定界符是开括号后的第一个符号。换句话说，开括号后跟定界符与定界符和闭括号之间的所有文本都被视为RE2表达式。
请注意，存储在字典中的词项会经过`charset_table`转换，这意味着例如，如果根据`charset_table`（默认情况）将所有字符转换为小写，则REGEX可能无法匹配大写字符。要成功使用REGEX表达式匹配词项，模式必须对应整个词项。要实现部分匹配，请在模式的开头和/或结尾放置`.*`。

```sql
REGEX(/.{3}t/)
REGEX(/t.*\d*/)
```

### 字段起始和字段结束修饰符

```sql
^hello world$
```

字段起始和字段结束关键字修饰符确保关键字仅在出现在全文字段的最开始或最末尾时匹配。例如，查询`"^hello world$"`（用引号括起来以将短语操作符与起始/结束修饰符结合）将仅匹配包含至少一个字段具有这两个特定关键字的文档。

### IDF提升修饰符

```sql
boosted^1.234 boostedfieldend$^1.234
```

提升修饰符通过指定的因子提升包含IDF计算的排名分数中的词项[IDF](../../Searching/Options.md#idf)_分数。它不会以任何方式影响匹配过程。

### NEAR操作符

```sql
hello NEAR/3 world NEAR/4 "my test"
```

`NEAR`操作符是邻近操作符的更通用版本。其语法为`NEAR/N`，区分大小写，且`NEAR`关键字、斜杠和距离值之间不允许有空格。

虽然原始邻近操作符仅适用于关键字集合，`NEAR`更灵活，可以接受任意子表达式作为其两个参数。当两个子表达式在文档中相距不超过N个词时，无论顺序如何，均匹配该文档。`NEAR`是左结合的，且与[BEFORE](../../Searching/Full_text_matching/Operators.md#Strict-order-operator)具有相同的（最低）优先级。

需要注意的是，`one NEAR/7 two NEAR/7 three`并不完全等同于`"one two three"~7`。关键区别在于邻近操作符允许在三个匹配词之间最多有6个不匹配词，而使用`NEAR`的版本限制较少：它允许`one`和`two`之间最多6个词，然后在该两个词匹配与`three`之间再允许最多6个词。

### NOTNEAR操作符

```sql
Church NOTNEAR/3 street
```
`NOTNEAR`操作符作为否定断言。当左参数存在且右参数不存在于文档中，或右参数与左匹配参数的结尾相距指定距离时，匹配该文档。距离以词数表示。语法为`NOTNEAR/N`，区分大小写，且`NOTNEAR`关键字、斜杠和距离值之间不允许有空格。该操作符的两个参数可以是词项，也可以是任何操作符或操作符组。

### SENTENCE和PARAGRAPH操作符

```sql
all SENTENCE words SENTENCE "in one sentence"
```


```sql
"Bill Gates" PARAGRAPH "Steve Jobs"
```
`SENTENCE` 和 `PARAGRAPH` 操作符分别在它们的两个参数位于同一句子或同一段文本内时匹配文档。这些参数可以是关键词、短语或相同操作符的实例。

参数在句子或段落中的顺序无关紧要。这些操作符仅在启用了 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)（句子和段落索引功能）的表中有效，否则退化为简单的 AND 操作。有关句子和段落的定义，请参阅 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp) 指令文档。


### ZONE 限定操作符

```sql
ZONE:(h3,h4)

only in these titles
```

`ZONE limit` 操作符与字段限定操作符非常相似，但将匹配限制在指定的字段内区域或区域列表中。需要注意的是，后续的子表达式不必在给定区域的单个连续范围内匹配，可以跨多个范围匹配。例如，查询 `(ZONE:th hello world)` 将匹配以下示例文档：

```html
<th>Table 1. Local awareness of Hello Kitty brand.</th>
.. some table data goes here ..
<th>Table 2. World-wide brand awareness.</th>
```

`ZONE` 操作符影响查询，直到遇到下一个字段或 `ZONE` 限定操作符，或直到闭合括号。它仅在支持区域的表中有效（参见 [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)），否则将被忽略。

### ZONESPAN 限定操作符

```sql
ZONESPAN:(h2)

only in a (single) title
```

`ZONESPAN` 限定操作符类似于 `ZONE` 操作符，但要求匹配必须发生在单个连续范围内。在前面给出的示例中，`ZONESPAN:th hello world` 不会匹配该文档，因为 "hello" 和 "world" 不在同一范围内。

<!-- proofread -->

