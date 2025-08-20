# 全文运算符

查询字符串可以包含定义查询词匹配条件的特定运算符。

### 布尔运算符

#### AND 运算符

隐含的逻辑 AND 运算符始终存在，因此 "hello world" 表示匹配文档中必须同时包含 "hello" 和 "world"。

```sql
hello  world
```

注意：没有显式的 `AND` 运算符。

#### OR 运算符

逻辑 OR 运算符 `|` 的优先级高于 AND，因此 `looking for cat | dog | mouse` 表示 `looking for (cat | dog | mouse)`，而不是 `(looking for cat) | dog | mouse`。

```sql
hello | world
```

注意：没有 `OR` 运算符，请使用 `|`。

### MAYBE 运算符

```sql
hello MAYBE world
```

`MAYBE` 运算符的功能与 `|` 运算符类似，但它不会返回仅匹配右子树表达式的文档。

### 否定运算符

```sql
hello -world
hello !world
```

否定运算符用于强制单词不存在。

仅包含否定的查询默认不被支持。要启用，请使用服务器选项 [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed)。

### 字段搜索运算符

```sql
@title hello @body world
```

字段限制运算符限制随后的搜索仅在指定字段内进行。默认情况下，如果给定的字段名在被搜索的表中不存在，查询将失败并返回错误消息。但可以通过在查询开头指定 `@@relaxed` 选项来禁止该行为：

```sql
@@relaxed @nosuchfield my query
```

这在搜索具有不同模式的异构表时很有用。

字段位置限制进一步约束搜索范围为字段（或多个字段）中的前 N 个位置。例如，`@body [50] hello` 不会匹配关键字 `hello` 出现在 body 字段第 51 个或更后位置的文档。

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

忽略多字段搜索运算符（如果有字段 'title'、'subject' 和 'body'，则 `@!(title)` 等价于 `@(subject,body)`）：

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

短语运算符要求词语彼此相邻。

短语搜索运算符可以包含“匹配任一词”修饰符。在短语运算符中，词的位置信息非常关键。使用“匹配任一词”修饰符时，该短语查询中后续词的位置会被调整。因此，“匹配任一词”修饰符不会影响搜索性能。

```sql
"exact * phrase * * for terms"
```

你也可以在引号内使用 OR 运算符。OR 运算符（`|`）在短语内必须用括号 `()` 括起来。每个选项都会在同一位置单独匹配，若任一选项匹配成功，整个短语匹配。

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

### 近邻搜索运算符

```sql
"hello world"~10
```

近邻距离以词数计，适用于引号内的所有词。例如，查询 `"cat dog mouse"~5` 表示必须有少于 8 个词的范围包含所有 3 个词。因此，包含 `CAT aaa bbb ccc DOG eee fff MOUSE` 的文档不会匹配此查询，因为该范围长达恰好 8 词。

你也可以在近邻搜索内使用 OR 运算符。OR 运算符（`|`）在近邻搜索内必须用括号 `()` 括起来。每个选项分开进行检查。

**正确示例**（带括号）：
```sql
"( two | four ) fish chips"~5
```

**错误示例**（无括号 - 不会生效）：
```sql
"two | four fish chips"~5
```

### 门槛匹配运算符

```sql
"the world is a wonderful place"/3
```

门槛匹配运算符引入了一种模糊匹配类型。它仅匹配满足指定词数门槛的文档。例如 (`"the world is a wonderful place"/3`) 会匹配至少包含指定的 6 个词中至少 3 个的所有文档。运算符支持最多 255 个关键词。你也可以提供一个 0.0 到 1.0 之间的值（表示 0% 到 100%），Manticore 将匹配包含给定词的至少该百分比的文档。上述示例也可以写作 `"the world is a wonderful place"/0.5`，匹配至少包含这 6 个词中 50% 的文档。

门槛运算符支持 OR (`|`) 运算符。OR 运算符（`|`）在门槛匹配中必须用括号 `()` 括起来。每组 OR 中只有一个词计入匹配。

**正确示例**（带括号）：
```sql
"( ( a b c ) | d ) e f g"/0.5
"happy ( sad | angry ) man"/2
```

**错误示例**（无括号 - 不会生效）：
```sql
"a b c | d e f g"/0.5
```

### 严格顺序运算符

```sql
aaa << bbb << ccc
```

严格顺序运算符（也称为“前置”运算符）只在查询指定的关键词严格按顺序出现在文档中时才匹配。例如，查询 `black << cat` 会匹配 "black and white cat" 但不会匹配 "that cat was black"。顺序运算符优先级最低。它可以应用于单个关键词，也可以应用于更复杂的表达式。例如，下面是一个有效的查询：

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### 精确形式修饰符

```sql
raining =cats and =dogs
="exact phrase"
```

精确形式关键词修饰符只匹配关键词以指定的精确形式出现的文档。默认情况下，如果词的词干/词形变化匹配，文档即被视为匹配。例如，查询 "runs" 会匹配包含 "runs" 和包含 "running" 的文档，因为两者的词干均为 "run"。但 `=runs` 查询只匹配前者。精确形式修饰符需要启用 [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 选项。


























































































































