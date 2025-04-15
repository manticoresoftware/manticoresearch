# 拼写纠正

拼写纠正，也称为：

* 自动纠正
* 文本纠正
* 修复拼写错误
* 错别字容忍
* “你是想说？” 

等等，是一种软件功能，可以为您输入的文本建议替代方案或进行自动纠正。纠正输入文本的概念可以追溯到20世纪60年代，当时计算机科学家沃伦·泰特尔曼（Warren Teitelman）引入了一种名为D.W.I.M.（“按我想的做”）的计算哲学，他也是“撤销”命令的发明者。泰特尔曼认为，不应该编程让计算机仅接受完美格式的指令，而应该使其被编程为识别明显的错误。

第一个提供拼写纠正功能的知名产品是1993年发布的Microsoft Word 6.0。

### 它是如何工作的

拼写纠正可以通过几种方式进行，但重要的是要注意，没有纯粹的程序化方法可以以合理的质量将您键入的“ipone”转换为“iphone”。大多数情况下，系统必须基于某个数据集。该数据集可以是：

* 一个正确拼写单词的词典，词典可以是：
  * 基于您的真实数据。这里的idea是，词典中的拼写在大多数情况下是正确的，系统尝试寻找与输入单词最相似的单词（我们将很快讨论如何使用Manticore来实现这一点）。
  * 或者它可以基于与您的数据无关的外部词典。这里可能出现的问题是，您的数据和外部词典可能差异过大：词典中可能缺少某些单词，而您的数据中也可能缺少某些单词。
* 不仅仅是基于词典，还具有上下文相关性，例如，“white ber”会被纠正为“white bear”，而“dark ber”会被纠正为“dark beer”。上下文不仅可以是查询中的相邻单词，还可以是您的位置、时间、当前句子的语法（是否更改“there”到“their”），您的搜索历史以及几乎任何其他可能影响您意图的因素。
* 另一个经典的方法是使用以前的搜索查询作为拼写纠正的数据集。这在[自动完成](../Searching/Autocomplete.md)功能中使用得更多，但对自动纠正也是有意义的。这个idea是，用户的拼写通常是正确的，因此我们可以同时使用他们的搜索历史中的单词作为真相来源，即使我们在文档中没有这些单词或没有使用外部词典。在这里，上下文相关性也是可能的。

Manticore提供模糊搜索选项和可用于自动拼写纠正的命令`CALL QSUGGEST`和`CALL SUGGEST`。

# 模糊搜索

模糊搜索功能允许通过考虑搜索查询中的轻微变体或拼写错误来实现更灵活的匹配。它的工作原理类似于普通的`SELECT` SQL语句或`/search` JSON请求，但提供了额外的参数来控制模糊匹配行为。

> 注意：`fuzzy`选项需要[ Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它无法正常工作，请确保Buddy已安装。

## 一般语法

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

注意：在通过SQL进行模糊搜索时，MATCH子句不应包含除[短语搜索运算符](../Searching/Full_text_matching/Operators.md#Phrase-search-operator)以外的任何全文操作符，并且应仅包含您意图匹配的单词。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM mytable WHERE MATCH('someting') OPTION fuzzy=1, layouts='us,ua', distance=2;
```

<!-- request SQL with additional filters -->
带有附加过滤器的更复杂的模糊搜索查询示例：

```sql
SELECT * FROM mytable WHERE MATCH('someting') OPTION fuzzy=1 AND (category='books' AND price < 20);
```

<!-- request JSON -->

```json
POST /search
{
  "table": "test",
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
  "table": "table_name",
  "query": {
    <全文本查询>
  },
  "options": {
    "fuzzy": {true|false}
    [,"layouts": ["be","bg","br","ch","de","dk","es","fr","uk","gr","it","no","pt","ru","se","ua","us"]]
    [,"distance": N]
  }
}
```

注意：如果使用[query_string](../../Searching/Full_text_matching/Basic_usage.md#query_string)，请注意它不支持除[短语搜索运算符](../Searching/Full_text_matching/Operators.md#Phrase-search-operator)以外的全文操作符。查询字符串应仅包含您希望匹配的单词。

### 选项

- `fuzzy`：开启或关闭模糊搜索。
- `distance`：设置匹配的莱文斯坦距离。默认值为`2`。
- `layouts`：检查输入错误的键盘布局。默认情况下使用所有布局。使用空字符串`''`（SQL）或数组`[]`（JSON）来关闭此功能。支持的布局包括：
  - `be` - 比利时AZERTY布局
  - `bg` - 标准保加利亚布局
  - `br` - 巴西QWERTY布局
  - `ch` - 瑞士QWERTZ布局
  - `de` - 德国QWERTZ布局
  - `dk` - 丹麦QWERTY布局
  - `es` - 西班牙QWERTY布局
  - `fr` - 法国AZERTY布局
  - `uk` - 英国 QWERTY 布局
  - `gr` - 希腊 QWERTY 布局
  - `it` - 意大利 QWERTY 布局
  - `no` - 挪威 QWERTY 布局
  - `pt` - 葡萄牙 QWERTY 布局
  - `ru` - 俄罗斯 JCUKEN 布局
  - `se` - 瑞典 QWERTY 布局
  - `ua` - 乌克兰 JCUKEN 布局
  - `us` - 美国 QWERTY 布局


### 链接

* <a href="https://github.manticoresearch.com/manticoresoftware/manticoresearch?query=fature&filters%5Bcomment%5D%5B%5D=28798446&filters%5Bcommon%5D%5Brepo_id%5D%5B%5D=95614931&sort=&search=keyword-search-fuzzy-layouts">这个演示</a> 演示了模糊搜索功能：
  ![Fuzzy search example](fuzzysearch.png){.scale-0.7}
* 关于模糊搜索和自动补全的博客 - https://manticoresearch.com/blog/new-fuzzy-search-and-autocomplete/

## 调用 QSUGGEST，调用 SUGGEST

这两个命令可以通过 SQL 访问，并支持查询本地（纯文本和实时）和分布式表。语法如下：
```sql
CALL QSUGGEST(<word or words>, <table name> [,options])
CALL SUGGEST(<word or words>, <table name> [,options])

options: N as option_name[, M as another_option, ...]
```

这些命令为给定单词提供字典中的所有建议。它们仅在启用 [infixing](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) 和 [dict=keywords](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) 的表上工作。它们返回建议的关键词、建议关键词和原关键词之间的 Levenshtein 距离，以及建议关键词的文档统计信息。

如果第一个参数包含多个单词，那么：
* `CALL QSUGGEST` 仅返回 **最后一个** 单词的建议，忽略其余部分。
* `CALL SUGGEST` 仅返回 **第一个** 单词的建议。

这就是它们之间的唯一区别。支持多个选项以进行自定义：

| 选项 | 描述 | 默认值 |
| - | - | - |
| limit | 返回 N 个最佳匹配 | 5 |
| max_edits | 仅保留 Levenshtein 距离小于或等于 N 的字典单词 | 4 |
| result_stats | 提供所找到单词的 Levenshtein 距离和文档数量 | 1 (启用) |
| delta_len | 仅保留长度差异小于 N 的字典单词 | 3 |
| max_matches | 保留的匹配数量 | 25 |
| reject | 被拒绝的词是匹配中不比已在匹配队列中的更好的词。它们被放入一个被拒绝队列，在实际可以进入匹配队列的情况下会被重置。此参数定义被拒绝队列的大小（作为 reject*max(max_matched,limit)）。如果被拒绝队列被填满，引擎将停止寻找潜在匹配 | 4 |
| result_line | 显示数据的替代模式，每行返回所有建议、距离和文档 | 0 |
| non_char | 不跳过具有非字母符号的字典单词 | 0 (跳过此类单词) |
| sentence | 返回原始句子，同时用匹配的单词替换最后一个单词。 | 0 (不返回完整句子) |

为了显示它是如何工作的，让我们创建一个表并向其中添加一些文档。

```sql
create table products(title text) min_infix_len='2';
insert into products values (0,'Crossbody Bag with Tassel'), (0,'microfiber sheet set'), (0,'Pet Hair Remover Glove');
```
<!-- example single -->
##### 单词示例
如你所见，错拼的单词 "crossb**U**dy" 被更正为 "crossbody"。默认情况下，`CALL SUGGEST/QSUGGEST` 返回：

* `distance` - Levenshtein 距离，这意味着将给定单词转换为建议所需的编辑次数
* `docs` - 包含建议单词的文档数量

要禁用这些统计信息的显示，可以使用选项 `0 as result_stats`。


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
##### CALL SUGGEST 仅取第一个单词
如果第一个参数不是单个单词，而是多个单词，那么 `CALL SUGGEST` 仅会返回第一个单词的建议。


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
##### CALL QSUGGEST 仅取最后一个单词
如果第一个参数不是单个单词，而是多个单词，那么 `CALL QSUGGEST` 仅会返回最后一个单词的建议。


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

添加 `1 as sentence` 使得 `CALL QSUGGEST` 返回整个句子并纠正最后一个单词。

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
`1 as result_line` 选项改变了建议在输出中的显示方式。它不会将每个建议显示在单独的行中，而是将所有的建议、距离和文档显示在一行中。以下是一个示例来演示这一点：

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

* [这个互动课程](https://play.manticoresearch.com/didyoumean/) 展示了 `CALL SUGGEST` 如何在一个小型网页应用中工作。

![CALL SUGGEST 示例](didyoumean.png){.scale-0.5}


<!-- proofread -->
