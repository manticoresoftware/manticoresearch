# 全文操作符

查询字符串可以包含特定的操作符，用于定义查询字符串中的词语应如何匹配的条件。

### 布尔操作符

#### AND 操作符

隐含的逻辑 AND 操作符总是存在，因此 "hello world" 意味着匹配的文档中必须同时包含 "hello" 和 "world"。

```sql
hello  world
```

注意：没有显式的 `AND` 操作符。

#### OR 操作符

逻辑 OR 操作符 `|` 的优先级高于 AND，所以 `looking for cat | dog | mouse` 表示的是 `looking for (cat | dog | mouse)`，而不是 `(looking for cat) | dog | mouse`。

```sql
hello | world
```

注意：没有 `OR` 操作符。请使用 `|` 替代。

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

否定操作符要求词语不出现在文档中。

默认情况下，不支持仅包含否定的查询。要启用此功能，请使用服务器选项 [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed)。

### 字段搜索操作符

```sql
@title hello @body world
```

字段限制操作符将后续搜索限制在指定字段内。默认情况下，如果指定的字段名在被搜索的表中不存在，查询将失败并返回错误消息。 但是，可以在查询开头指定 `@@relaxed` 选项以抑制此行为：

```sql
@@relaxed @nosuchfield my query
```

这在搜索具有不同模式的异构表时非常有用。

字段位置限制进一步将搜索约束在给定字段（或字段组）中的前 N 个位置。例如，`@body [50] hello` 不会匹配关键字 `hello` 出现在正文中第 51 个位置或更后位置的文档。

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

短语操作符要求词语必须相邻出现。

短语搜索操作符可以包含 `match any term` 修饰符。在短语操作符内部，词语的位置是具有意义的。当使用“匹配任一词语”修饰符时，该短语查询中后续词语的位置将被移动。因此，“匹配任一”修饰符不会影响搜索性能。

```sql
"exact * phrase * * for terms"
```

你也可以在引号内部使用 OR 操作符。OR 操作符 (`|`) 在短语内部使用时，必须用括号 `()` 包围。每个选项都在同一位置被检查，只要有任何选项匹配该位置，短语就匹配。

**正确例子**（带括号）：
```sql
"( a | b ) c"
"( ( a b c ) | d ) e"
"man ( happy | sad ) but all ( ( as good ) | ( as fast ) )"
```

**错误例子**（无括号 - 这些不会生效）：
```sql
"a | b c"
"happy | sad"
```

###  邻近搜索操作符

```sql
"hello world"~10
```

邻近距离以词数衡量，考虑单词数量，并应用于引号中的所有词。例如，查询 `"cat dog mouse"~5` 表示必须存在一个少于 8 个单词的区间包含全部三个词。因此，含有 `CAT aaa bbb ccc DOG eee fff MOUSE` 的文档不会匹配此查询，因为区间恰好是 8 个单词。

你也可以在邻近搜索内使用 OR 操作符。OR 操作符 (`|`) 在邻近搜索内使用时，必须用括号 `()` 包围。每个选项会单独检查。

**正确例子**（带括号）：
```sql
"( two | four ) fish chips"~5
```

**错误例子**（无括号 - 这不会生效）：
```sql
"two | four fish chips"~5
```

###  成年匹配操作符

```sql
"the world is a wonderful place"/3
```

成年匹配操作符引入了一种模糊匹配方式。它只匹配满足指定词语阈值的文档。例如，前面 `"the world is a wonderful place"/3` 会匹配包含指定的 6 个词中至少 3 个的所有文档。该操作符限制最多255个关键词。你也可以提供一个介于 0.0 和 1.0 之间的值（代表 0% 和 100%），Manticore 将只匹配包含至少该百分比词语的文档。上述示例也可以写作 `"the world is a wonderful place"/0.5`，它会匹配至少包含 6 个词中 50% 的文档。

成年操作符支持 OR (`|`) 操作符。在成年匹配中使用 OR 操作符 (`|`) 时，必须用括号 `()` 包围。每个 OR 组中只计算一个词算作匹配的一部分。

**正确例子**（带括号）：
```sql
"( ( a b c ) | d ) e f g"/0.5
"happy ( sad | angry ) man"/2
```

**错误例子**（无括号 - 这不会生效）：
```sql
"a b c | d e f g"/0.5
```

### 严格顺序操作符

```sql
aaa << bbb << ccc
```

严格顺序操作符（也称为“之前”操作符）只会匹配其参数关键词在文档中完全按照查询中指定顺序出现的文档。例如，查询 `black << cat` 会匹配文档 "black and white cat"，但不匹配文档 "that cat was black"。顺序操作符优先级最低。它既可应用于单个关键词，也可应用于更复杂的表达式。例如，以下是一个有效的查询：

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### 精确形式修饰符

```sql
raining =cats and =dogs
="exact phrase"
```

精确形式关键词修饰符只会匹配关键词以查询中指定的精确形式出现的文档。默认情况下，如果词根化/词形还原的关键词匹配，文档就被视为匹配。例如，查询 "runs" 会匹配包含 "runs" 和包含 "running" 的文档，因为两种形式词根化后都是 "run"。但 `=runs` 查询只会匹配第一种文档。精确形式修饰符需要启用 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 选项。

另一个用例是防止将关键词[扩展](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)为其`*keyword*`形式。例如，使用`index_exact_words=1` + `expand_keywords=1/star`时，`bcd`可以找到包含`abcde`的文档，但`=bcd`不能。

作为影响关键词的修饰符，它可以在短语、邻近和法定人数等操作符内使用。短语操作符应用精确形式修饰符是可能的，在这种情况下，它会在内部将精确形式修饰符添加到短语中的所有词项。

### 通配符操作符

```sql
nation* *nation* *national
```

需要前缀（尾部扩展）和/或后缀（头部扩展）的[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。如果仅需要前缀，可以改用[min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)。

搜索将尝试找到通配符词项的所有扩展项，每个扩展项都会被记录为匹配命中。可以用[expansion_limit](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit)表设置控制词项的扩展数量。通配符词项对查询搜索时间有显著影响，尤其是词项较短时。在这种情况下，建议使用扩展限制。

如果使用[expand_keywords](../../Searching/Options.md#expand_keywords)表设置，通配符操作符可以自动应用。

此外，支持以下内联通配符操作符：

* `?` 可以匹配任何单个字符：`t?st`将匹配`test`，但不匹配`teast`
* `%` 可以匹配零个或一个字符：`tes%`将匹配`tes`或`test`，但不匹配`testing`

内联操作符需要`dict=keywords`（默认启用）和启用前缀/中缀。

### REGEX操作符

```sql
REGEX(/t.?e/)
```

需要设置[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)或[min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)和[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords选项（默认已设置）。

类似于[wildcard operators](../../Searching/Full_text_matching/Operators.md#Wildcard-operators)，REGEX操作符尝试找到所有匹配给定模式的词项，每个扩展都被记录为匹配命中。注意，这会对查询搜索时间产生显著影响，因为要扫描整个字典，并对字典中的每个词项进行REGEX匹配。

模式应遵循[RE2语法](https://github.com/google/re2/wiki/Syntax)。REGEX表达式的定界符是开括号后的第一个符号。换句话说，开括号后跟定界符和定界符后紧跟闭括号之间的所有文本都视为RE2表达式。
请注意，存储在字典中的词项经过`charset_table`转换，意味着例如，如果根据`charset_table`（默认情况）所有字符均转换为小写后，REGEX可能无法匹配大写字符。要成功使用REGEX匹配词项，模式必须对应整个词项。若要实现部分匹配，请在模式的开头和/或结尾加上`.*`。

```sql
REGEX(/.{3}t/)
REGEX(/t.*\d*/)
```

### 字段起始和字段结束修饰符

```sql
^hello world$
```

字段起始和字段结束关键词修饰符分别确保关键词仅在全文字段的最开始或最末尾出现时匹配。例如，查询`"^hello world$"`（用引号括起以结合短语操作符和起始/结束修饰符）将仅匹配至少包含包含这两个特定关键词的字段的文档。

### IDF提升修饰符

```sql
boosted^1.234 boostedfieldend$^1.234
```

提升修饰符通过指定的因子提升包含IDF计算的排名分数中的词项[IDF](../../Searching/Options.md#idf)_得分。它不影响匹配过程。

### NEAR操作符

```sql
hello NEAR/3 world NEAR/4 "my test"
```

`NEAR`操作符是邻近操作符的更通用版本。其语法为`NEAR/N`，区分大小写，且`NEAR`关键字、斜杠和距离值之间不允许有空格。

原邻近操作符仅对关键词集合有效，而`NEAR`更灵活，两个参数可以是任意子表达式。它在两个子表达式在N个词以内出现时匹配文档，不考虑顺序。`NEAR`是左结合的，且与[BEFORE](../../Searching/Full_text_matching/Operators.md#Strict-order-operator)具有相同的（最低）优先级。

需要注意的是，`one NEAR/7 two NEAR/7 three`与`"one two three"~7`不完全等价。关键区别在于邻近操作符允许三个匹配词之间最多有6个不匹配词，而带`NEAR`的版本限制较宽松：允许`one`与`two`之间最多6个词，然后该两个词匹配与`three`之间再最多6个词。

### NOTNEAR操作符

```sql
Church NOTNEAR/3 street
```
`NOTNEAR`操作符作为否定断言使用。当左参数存在且右参数要么不存在，要么距离左匹配参数结束位置指定距离时匹配文档。距离以词数计。语法为`NOTNEAR/N`，区分大小写，不允许`NOTNEAR`关键字、斜杠和距离值之间有空格。该操作符的两个参数可以是词项或任意操作符或操作符组。

### SENTENCE和PARAGRAPH操作符

```sql
all SENTENCE words SENTENCE "in one sentence"
```


```sql
"Bill Gates" PARAGRAPH "Steve Jobs"
```
`SENTENCE` 和 `PARAGRAPH` 操作符在它们的两个参数都位于同一句子或同一段文本中时匹配文档。这些参数可以是关键字、短语或同一操作符的实例。

参数在句子或段落中的顺序无关紧要。这些操作符仅适用于启用了 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)（句子和段落索引功能）的表，否则将回退为简单的 AND 操作。有关什么构成句子和段落的信息，请参阅 [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp) 指令文档。


### ZONE 限制操作符

```sql
ZONE:(h3,h4)

only in these titles
```

`ZONE limit` 操作符与字段限制操作符非常相似，但将匹配限制在指定的字段内区域或区域列表中。需要注意的是，后续的子表达式不需要在给定区域的单个连续跨度内匹配，可能跨多个跨度进行匹配。例如，查询 `(ZONE:th hello world)` 将匹配以下示例文档：

```html
<th>Table 1. Local awareness of Hello Kitty brand.</th>
.. some table data goes here ..
<th>Table 2. World-wide brand awareness.</th>
```

`ZONE` 操作符影响查询，直到下一个字段或 `ZONE` 限制操作符，或直到闭合括号。它仅适用于支持区域的表（参见 [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)），否则将被忽略。

### ZONESPAN 限制操作符

```sql
ZONESPAN:(h2)

only in a (single) title
```

`ZONESPAN` 限制操作符类似于 `ZONE` 操作符，但要求匹配发生在单个连续跨度内。以上示例中，`ZONESPAN:th hello world` 不会匹配文档，因为 "hello" 和 "world" 不出现在同一跨度内。

<!-- proofread -->

