# 拼写校正

拼写校正也被称为：

- 自动校正
- 文字校正
- 修正拼写错误
- 错误容忍度
- “您是想说...？”

等，是一种软件功能，旨在为您输入的文本提供替代建议或进行自动修正。拼写校正的概念可以追溯到1960年代，当时计算机科学家 Warren Teitelman（也发明了“撤销”命令）提出了一种名为 D.W.I.M.（Do What I Mean，做我想做的）的计算哲学。Teitelman 认为，计算机不应只接受格式完全正确的指令，而应被编程为能识别明显的错误。

第一个众所周知的提供拼写校正功能的产品是微软在1993年发布的 Word 6.0。

### 工作原理

拼写校正可以通过几种方式实现，但需要注意的是，没有纯编程的方法可以高质量地将输入错误的“ipone”转换为“iphone”。拼写校正通常需要一个数据集，系统可以基于该数据集进行处理。数据集可以是：

- 正确拼写单词的字典，该字典可以：
  - 基于您的实际数据。这个思路是，在大多数情况下，基于您的数据构建的字典拼写是正确的，系统会尝试找到与输入单词最相似的单词（稍后我们将讨论如何使用 Manticore 来实现此目的）。
  - 或者可以基于与您的数据无关的外部字典。这里可能会出现的问题是，您的数据和外部字典可能相差太大：字典中可能缺少某些单词，而您的数据中也可能缺少一些字典中的词。
- 不仅仅基于字典，还可以基于上下文，例如，“white ber”会被修正为“white bear”，而“dark ber”会被修正为“dark beer”。上下文不仅可能是查询中的邻近单词，还可以是您的位置、时间、当前句子的语法（是否将“there”改为“their”），您的搜索历史以及几乎任何其他可能影响您意图的因素。
- 另一种经典的方法是使用先前的搜索查询作为拼写校正的数据集。这在[自动补全](../Searching/Autocomplete.md)功能中得到了更多应用，但在自动校正中也同样有意义。其思想是，用户的拼写大多数时候是正确的，因此我们可以使用他们的搜索历史中的词汇作为事实依据，即使我们的文档中没有这些词，或者我们使用了外部字典。上下文感知在这里也可以起作用。

Manticore 提供了模糊搜索选项，并且可以使用 `CALL QSUGGEST` 和 `CALL SUGGEST` 命令来实现自动拼写校正。

# 模糊搜索

模糊搜索功能允许在匹配时考虑轻微的拼写变化或错误。它与正常的 `SELECT` SQL 语句或 `/search` JSON 请求类似，但提供了额外的参数以控制模糊匹配行为。

> 注意：`fuzzy` 选项需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

## 通用语法

### SQL

<!-- example Fuzzy_Search_SQL -->

```sql
SELECT
  ...
  MATCH('...')
  ...
  OPTION fuzzy={0|1}
  [, distance=N]
  [, layouts='{be,bg,br,ch,de,dk,es,fr,uk,gr,it,no,pt,ru,se,ua,us}']
}
```

注意：通过 SQL 进行模糊搜索时，`MATCH` 子句不应包含任何全文操作符，仅应包含要匹配的单词。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM mytable WHERE MATCH('someting') OPTION fuzzy=1, layouts='us,ua', distance=2;
```

<!-- request SQL with additional filters -->
带有额外过滤器的更复杂模糊搜索查询示例：

```sql
SELECT * FROM mytable WHERE MATCH('someting') OPTION fuzzy=1 AND (category='books' AND price < 20);
```

<!-- request JSON -->

```json
POST /search
{
  "index": "test",
  "query": {
    "bool": {
      "must": [
        {
          "match": {
            "*": "ghbdtn"
          }
        }
      ]
    }
  },
  "options": {
    "fuzzy": true,
    "layouts": ["us", "ru"],
    "distance": 2
  }
}
```

<!-- response SQL -->

```sql
+------+-------------+
| id   | content     |
+------+-------------+
|    1 | something   |
|    2 | some thing  |
+------+-------------+
2 rows in set (0.00 sec)
```

<!-- end -->

### JSON

```json
POST /search
{
  "index": "table_name",
  "query": {
    <full-text query>
  },
  "options": {
    "fuzzy": {true|false}
    [,"layouts": ["be","bg","br","ch","de","dk","es","fr","uk","gr","it","no","pt","ru","se","ua","us"]]
    [,"distance": N]
  }
}
```

注意：如果使用 [query_string](../Searching/Full_text_matching/Basic_usage.md#query_string)，请注意它不支持全文操作符。查询字符串应仅包含您希望匹配的单词。

### 选项

- `fuzzy`：打开或关闭模糊搜索。
- `distance`：设置用于匹配的 Levenshtein 距离。默认值为 `2`。
- `layouts`：检查键盘布局是否存在输入错误。默认情况下，使用所有布局。使用空字符串 `''` (SQL) 或数组 `[]` (JSON) 关闭此功能。支持的布局包括：
  
  - `be` - 比利时 AZERTY 布局
  - `bg` - 标准保加利亚布局
  - `br` - 巴西 QWERTY 布局
  - `ch` - 瑞士 QWERTZ 布局
  - `de` - 德国 QWERTZ 布局
  - `dk` - 丹麦 QWERTY 布局
  - `es` - 西班牙 QWERTY 布局
  - `fr` - 法国 AZERTY 布局
  - `uk` - 英国 QWERTY 布局
  - `gr` - 希腊 QWERTY 布局
  - `it` - 意大利 QWERTY 布局
  - `no` - 挪威 QWERTY 布局
  - `pt` - 葡萄牙 QWERTY 布局
  - `ru` - 俄语 JCUKEN 布局
  - `se` - 瑞典 QWERTY 布局
  - `ua` - 乌克兰 JCUKEN 布局
  - `us` - 美国 QWERTY 布局


### 链接

* <a href="https://github.manticoresearch.com/manticoresoftware/manticoresearch?query=fature&filters%5Bcomment%5D%5B%5D=28798446&filters%5Bcommon%5D%5Brepo_id%5D%5B%5D=95614931&sort=&search=keyword-search-fuzzy-layouts">这个演示</a> 展示了模糊搜索功能：
  ![Fuzzy search example](fuzzysearch.png){.scale-0.7}
* 关于模糊搜索和自动补全的博客文章 - https://manticoresearch.com/blog/new-fuzzy-search-and-autocomplete/

## CALL QSUGGEST, CALL SUGGEST

这两个命令可以通过 SQL 使用，并支持查询本地（普通和实时）和分布式表。语法如下：
```sql
CALL QSUGGEST(<word or words>, <table name> [,options])
CALL SUGGEST(<word or words>, <table name> [,options])

options: N as option_name[, M as another_option, ...]
```

这些命令根据给定单词从字典中提供所有建议。它们仅适用于启用了 [infixing](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 和 [dict=keywords](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 的表。返回建议的关键字、建议的关键字与原始关键字之间的 Levenshtein 距离以及建议关键字的文档统计。

如果第一个参数包含多个单词，则：

* `CALL QSUGGEST` 只返回**最后一个**单词的建议，忽略其余的单词。
* `CALL SUGGEST` 只返回**第一个**单词的建议。

这就是它们之间的唯一区别。支持几个选项用于自定义：

| 选项         | 描述                                                         | 默认值             |
| ------------ | ------------------------------------------------------------ | ------------------ |
| limit        | 返回前 N 个匹配项                                            | 5                  |
| max_edits    | 仅保留 Levenshtein 距离小于或等于 N 的字典单词               | 4                  |
| result_stats | 提供找到的单词的 Levenshtein 距离和文档数量                  | 1 (启用)           |
| delta_len    | 仅保留长度差小于 N 的字典单词                                | 3                  |
| max_matches  | 要保留的匹配数                                               | 25                 |
| reject       | 被拒绝的单词是那些不优于已在匹配队列中的单词的匹配项。它们会放入一个被拒绝的队列中，如果实际有一个单词可以进入匹配队列，则重置该队列。该参数定义了被拒绝队列的大小（作为 reject*max(max_matched,limit)）。如果被拒绝队列已满，系统将停止查找潜在匹配项 | 4                  |
| result_line  | 显示数据的替代模式，按每行返回所有建议、距离和文档数量       | 0                  |
| non_char     | 不跳过带有非字母符号的字典单词                               | 0 (跳过此类单词)   |
| sentence     | 返回原句并将最后一个单词替换为匹配的单词                     | 0 (不返回完整句子) |

为了展示其工作原理，让我们创建一个表并向其中添加一些文档。

```sql
create table products(title text) min_infix_len='2';
insert into products values (0,'Crossbody Bag with Tassel'), (0,'microfiber sheet set'), (0,'Pet Hair Remover Glove');
```
<!-- example single -->
##### 单词示例
如您所见，拼写错误的单词 "crossb**U**dy" 被更正为 "crossbody"。默认情况下，`CALL SUGGEST/QSUGGEST` 返回：

- `distance` - Levenshtein 距离，表示将给定单词转换为建议单词所需的编辑次数
- `docs` - 包含建议单词的文档数量

要禁用这些统计信息的显示，您可以使用选项 `0 as result_stats`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```sql
call suggest('crossbudy', 'products');
```
<!-- response Example -->

```sql
+-----------+----------+------+
| suggest   | distance | docs |
+-----------+----------+------+
| crossbody | 1        | 1    |
+-----------+----------+------+
```
<!-- end -->
<!-- example first -->
##### CALL SUGGEST takes only the first word
如果第一个参数不是一个单词而是多个单词，那么 `CALL SUGGEST` 只会返回**第一个单词**的建议。


<!-- intro -->
##### 示例：

<!-- request Example -->

```sql
call suggest('bagg with tasel', 'products');
```
<!-- response Example -->

```sql
+---------+----------+------+
| suggest | distance | docs |
+---------+----------+------+
| bag     | 1        | 1    |
+---------+----------+------+
```
<!-- end -->
<!-- example last -->
##### CALL QSUGGEST takes only the last word
如果第一个参数不是一个单词而是多个单词，那么 `CALL SUGGEST` 只会返回**最后一个单词**的建议。


<!-- intro -->
##### 示例：

<!-- request Example -->

```sql
CALL QSUGGEST('bagg with tasel', 'products');
```
<!-- response Example -->

```sql
+---------+----------+------+
| suggest | distance | docs |
+---------+----------+------+
| tassel  | 1        | 1    |
+---------+----------+------+
```
<!-- end -->

<!-- example last2 -->

添加 `1 as sentence` 会使 `CALL QSUGGEST` 返回整个句子，并将最后一个单词更正。

<!-- request Example -->
```sql
CALL QSUGGEST('bag with tasel', 'products', 1 as sentence);
```
<!-- response Example -->
```sql
+-------------------+----------+------+
| suggest           | distance | docs |
+-------------------+----------+------+
| bag with tassel   | 1        | 1    |
+-------------------+----------+------+
```
<!-- end -->

##### 不同的显示模式
`1 as result_line` 选项改变了输出中建议的显示方式。它不再将每个建议显示在单独的一行中，而是将所有建议、距离和文档统计信息显示在一行中。以下是一个示例来演示这一点：

<!-- intro -->

##### 示例：

<!-- request Example -->

```sql
CALL QSUGGEST('bagg with tasel', 'products', 1 as result_line);
```
<!-- response Example -->

```sql
+----------+--------+
| name     | value  |
+----------+--------+
| suggests | tassel |
| distance | 1      |
| docs     | 1      |
+----------+--------+
```
<!-- end -->

### 演示

* [这个互动课程](https://play.manticoresearch.com/didyoumean/) 展示了 `CALL SUGGEST` 如何在一个小型 Web 应用中工作。

![CALL SUGGEST example](didyoumean.png){.scale-0.5}


<!-- proofread -->
