# 完整文本操作符

查询字符串可以包含特定操作符，用于定义查询字符串中的词应如何匹配的条件。

### 布尔操作符

#### AND 操作符

隐式的逻辑 AND 操作符始终存在，因此“hello world”表示必须在匹配的文档中同时找到“hello”和“world”。

```sql
hello  world
```

注意：没有显式的 `AND` 操作符。

#### OR 操作符

逻辑 OR 操作符 `|` 的优先级高于 AND，因此 `looking for cat | dog | mouse` 表示 `looking for (cat | dog | mouse)` 而不是 `(looking for cat) | dog | mouse`。

```sql
hello | world
```

注意：没有操作符 `OR`。请使用 `|`。

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

否定操作符强制某个词不存在的规则。

默认情况下，不支持只包含否定的查询。要启用此功能，请使用服务器选项 [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed)。

### 字段搜索操作符

```sql
@title hello @body world
```

字段限制操作符将后续搜索限制在指定字段中。默认情况下，如果给定的字段名称在被搜索的表中不存在，查询会失败并显示错误信息。但是，可以在查询开头指定 `@@relaxed` 选项来抑制此行为：

```sql
@@relaxed @nosuchfield my query
```

这在搜索具有不同模式的异构表时非常有用。

字段位置限制进一步约束搜索仅限于给定字段（或字段集）中的前 N 个位置。例如，`@body [50] hello` 不会匹配在正文中关键词 `hello` 出现在第 51 位置或之后的文档。

```sql
@body[50] hello
```

多字段搜索操作符：

```sql
@(title,body) hello world
```

忽略字段搜索操作符（忽略来自 `title` 字段中“hello world”的任何匹配）：

```sql
@!title hello world
```

忽略多字段搜索操作符（如果存在 `title`、`subject` 和 `body` 字段，则 `@!(title)` 等价于 `@(subject,body)`）：

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

短语操作符要求词语彼此相邻。

短语搜索操作符可以包含“匹配任一词”的修饰符。在短语操作符中，词语的位置具有重要意义。当使用“匹配任一词”修饰符时，该短语查询中之后的词语位置会被移动。因此，“匹配任一”修饰符不会影响搜索性能。

```sql
"exact * phrase * * for terms"
```

###  临近搜索操作符

```sql
"hello world"~10
```

临近距离以词数计数，针对引号内的所有词。例如，查询 `"cat dog mouse"~5` 表示必须有一个包含所有三个词的词距小于 8 的范围。因此，含有 `CAT aaa bbb ccc DOG eee fff MOUSE` 的文档不会匹配该查询，因为该范围正好是 8 个词长。

###  阈值匹配操作符

```sql
"the world is a wonderful place"/3
```

阈值匹配操作符引入了一种模糊匹配。它只匹配满足指定词数阈值的文档。在上述例子 (`"the world is a wonderful place"/3`) 中，它会匹配包含至少 6 个指定词中 3 个的所有文档。该操作符限定最多 255 个关键字。除了绝对数值外，还可以提供介于 0.0 到 1.0 之间的值（表示 0% 到 100%），Manticore 将只匹配包含给定词汇至少指定百分比的文档。上述例子也可以写成 `"the world is a wonderful place"/0.5`，会匹配包含至少 6 个词中 50% 的文档。

### 严格顺序操作符

```sql
aaa << bbb << ccc
```

严格顺序操作符（也称为“之前”操作符）仅匹配其参数关键词严格按照查询指定顺序出现在文档中的文档。例如，查询 `black << cat` 会匹配“black and white cat”，但不会匹配“that cat was black”。顺序操作符优先级最低。它可应用于单个关键词和更复杂的表达式。例如，这是一条有效的查询：

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### 精确形式修饰符

```sql
raining =cats and =dogs
="exact phrase"
```

精确形式关键词修饰符仅当关键词以指定的精确形式出现时匹配文档。默认情况下，只要词根/词形还原后的关键词匹配，即认为文档匹配。例如，查询“runs”会匹配包含“runs”和“running”的文档，因为这两个形态的词干都是“run”。但查询 `=runs` 仅匹配包含“runs”的文档。精确形式修饰符需要启用 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 选项。

另一个用例是防止将关键词扩展为其 `*keyword*` 形式。例如，启用 `index_exact_words=1` + `expand_keywords=1/star` 时，`bcd` 会找到包含 `abcde` 的文档，但 `=bcd` 不会。

作为影响关键词的修饰符，它可以与短语、临近和阈值操作符等一起使用。可以将精确形式修饰符应用于短语操作符，内部会将精确形式修饰符添加到短语中所有词条。

### 通配符操作符

```sql
nation* *nation* *national
```

需要使用 [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 来支持前缀（尾部扩展）和/或后缀（头部扩展）。如果只需要前缀匹配，可以改用 [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)。

搜索将尝试查找通配符标记的所有扩展，每个扩展都会被记录为一个匹配命中。标记的扩展数量可以通过[expansion_limit](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit)表设置来控制。通配符标记可能对查询搜索时间产生显著影响，尤其当标记长度较短时。在这种情况下，建议使用扩展限制。

如果使用了[expand_keywords](../../Searching/Options.md#expand_keywords)表设置，通配符操作符可以自动应用。

此外，支持以下内联通配符操作符：

* `?` 可以匹配任意单个字符：`t?st` 将匹配 `test`，但不匹配 `teast`
* `%` 可以匹配零个或一个字符：`tes%` 将匹配 `tes` 或 `test`，但不匹配 `testing`

内联操作符需要 `dict=keywords`（默认启用）和前缀/中缀支持启用。

### REGEX 操作符

```sql
REGEX(/t.?e/)
```

需要设置[min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)或[min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)以及[dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords选项（此为默认设置）。

和[wildcard operators](../../Searching/Full_text_matching/Operators.md#Wildcard-operators)类似，REGEX操作符尝试查找匹配提供的模式的所有标记，每个扩展都会被记录为匹配命中。请注意，这可能对查询搜索时间产生显著影响，因为需要扫描整个字典，并且字典中的每个词项都需要与REGEX模式进行匹配。

模式应遵循[RE2 语法](https://github.com/google/re2/wiki/Syntax)。REGEX表达式的定界符是开括号后的第一个符号。换句话说，开括号后跟定界符和定界符与闭括号之间的所有文本都被视为一个RE2表达式。
请注意，存储在字典中的词项会经过 `charset_table` 转换，意味着例如，如果所有字符根据 `charset_table`（默认情况下发生）被转换为小写，REGEX可能无法匹配大写字符。为了使用REGEX表达式成功匹配一个词项，模式必须对应整个标记。要实现部分匹配，请在模式的开头和/或结尾放置`.*`。

```sql
REGEX(/.{3}t/)
REGEX(/t.*\d*/)
```

### 字段起始和字段结束修饰符

```sql
^hello world$
```

字段起始和字段结束关键字修饰符确保只有当关键字出现在全文字段的开头或结尾时才匹配。比如，查询 `"^hello world$"`（用引号括起来以将短语操作符与起始/结束修饰符组合使用）将专门匹配包含至少一个字段拥有这两个特定关键字的文档。

### IDF 提升修饰符

```sql
boosted^1.234 boostedfieldend$^1.234
```

提升修饰符通过指定的因子提高包含IDF计算的排名评分中的词语 [IDF](../../Searching/Options.md#idf)_score。它不会以任何方式影响匹配过程。

### NEAR 操作符

```sql
hello NEAR/3 world NEAR/4 "my test"
```

`NEAR` 操作符是邻近操作符的一个更通用版本。其语法为 `NEAR/N`，区分大小写，并且在 `NEAR` 关键字、斜杠符号和距离值之间不允许有空格。

虽然原始的邻近操作符只能作用于关键字集合，`NEAR` 更灵活，可以接受任意子表达式作为其两个参数。当两个子表达式在文档中的词语距离不超过N个词时（无论顺序如何），则匹配该文档。`NEAR` 是左结合的，且与[BEFORE](../../Searching/Full_text_matching/Operators.md#Strict-order-operator)具有相同的（最低）优先级。

需要注意的是，`one NEAR/7 two NEAR/7 three` 并不完全等同于 `"one two three"~7`。关键区别在于邻近操作符允许在三个匹配词之间有最多6个非匹配词，而`NEAR`版本限制较少：它允许 `one` 和 `two` 之间最多有6个词，然后从这两个词匹配的位置到 `three` 之间再最多允许6个词。

### NOTNEAR 操作符

```sql
Church NOTNEAR/3 street
```
`NOTNEAR` 操作符用作否定断言。当左参数存在且右参数要么不存在于文档中，要么距离左匹配参数的结尾达到指定的词距时匹配该文档。距离以词数表示。语法为 `NOTNEAR/N`，区分大小写，且不允许在 `NOTNEAR` 关键字、斜杠符号和距离值之间有空格。该操作符的两个参数可以是词项，也可以是任何操作符或操作符组合。

### SENTENCE 和 PARAGRAPH 操作符

```sql
all SENTENCE words SENTENCE "in one sentence"
```


```sql
"Bill Gates" PARAGRAPH "Steve Jobs"
```
`SENTENCE` 和 `PARAGRAPH` 操作符在两个参数均位于同一句子或同一段落的文本中时匹配文档。这些参数可以是关键字、短语或同一操作符的实例。

参数在句子或段落中的顺序无关紧要。这些操作符仅适用于启用了[index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)（句子和段落索引功能）的表，否则退化为简单的 AND 操作。有关什么构成句子和段落的信息，请参见[index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)指令文档。


### ZONE 限制操作符

```sql
ZONE:(h3,h4)

only in these titles
```

`ZONE limit` 操作符与字段限制操作符非常相似，但它限制匹配到指定的字段内区域或区域列表。需要注意的是，后续的子表达式不必在给定区域的单个连续片段内匹配，可以跨多个片段匹配。例如，查询 `(ZONE:th hello world)` 将匹配以下示例文档：

```html
<th>Table 1. Local awareness of Hello Kitty brand.</th>
.. some table data goes here ..
<th>Table 2. World-wide brand awareness.</th>
```

`ZONE` 操作符影响查询直到下一个字段或 `ZONE` 限制操作符，或直到闭合括号。它仅对支持区域的表（参见 [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)）有效，否则将被忽略。

### ZONESPAN 限制操作符

```sql
ZONESPAN:(h2)

only in a (single) title
```

`ZONESPAN` 限制操作符类似于 `ZONE` 操作符，但要求匹配发生在单一连续的片段内。在前面提供的示例中，`ZONESPAN:th hello world` 不会匹配该文档，因为 "hello" 和 "world" 不在同一片段内。

<!-- proofread -->

