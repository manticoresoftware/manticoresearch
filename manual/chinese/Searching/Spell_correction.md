# 拼写校正

拼写校正，也被称为：

* 自动校正
* 文字校正
* 修正拼写错误
* 容错输入
* “您的意思是？”

等等，是一种软件功能，建议对您输入的文本提供替代项或自动纠正。纠正输入文本的概念可以追溯到20世纪60年代，当时计算机科学家沃伦·泰特尔曼（Warren Teitelman），也是“撤销”命令的发明者，引入了一种称为D.W.I.M.（“做我意思的事”）的计算理念。泰特尔曼认为，计算机不应仅接受格式完全正确的指令，而应被编程为能够识别明显的错误。

第一个广为人知的具有拼写校正功能的产品是1993年发布的 Microsoft Word 6.0。

### 工作原理

拼写校正的实现方法有几种，但重要的是要注意，没有纯程序化的方法能高质量地将错误输入的“ipone”转换为“iphone”。主要是系统需要基于某个数据集。数据集可以是：

* 一个正确拼写单词的词典，词典可以是：
  * 基于您的真实数据。这里的思想是，词典中由您的数据构成的拼写在大多数情况下是正确的，系统尝试找到与输入单词最相似的单词（我们稍后将讨论如何用 Manticore 实现）。
  * 或者它可以基于与您的数据无关的外部词典。这里可能出现的问题是您的数据与外部词典差异过大：词典中可能缺少一些单词，而您的数据中可能缺少其他单词。
* 不仅基于词典，还具有上下文感知能力，例如，“white ber”会被校正为“white bear”，而“dark ber”会被校正为“dark beer”。上下文不仅可能是查询中相邻的词，还可能包括您的位置、时段、当前句子的语法（决定是否将“there”改为“their”）、您的搜索历史，以及几乎任何可能影响您意图的因素。
* 另一种经典方法是使用之前的搜索查询作为拼写校正的数据集。这在[自动完成](../Searching/Autocomplete.md)功能中使用得更多，但对自动更正同样有意义。其思想是用户的拼写大多是正确的，因此我们可以使用他们搜索历史中的单词作为真实来源，即使我们文档中没有这些单词，或者不使用外部词典。这里同样可能实现上下文感知。

Manticore 提供模糊搜索选项以及命令 `CALL QSUGGEST` 和 `CALL SUGGEST`，可用于自动拼写校正。

## 模糊搜索

模糊搜索功能允许更灵活的匹配，考虑搜索查询中的轻微变体或拼写错误。它的工作方式类似于普通的 `SELECT` SQL 语句或 `/search` JSON 请求，但提供了额外参数来控制模糊匹配行为。

> 注意：`fuzzy` 选项需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保 Buddy 已安装。

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
  [, preserve={0|1}]
  [, layouts='{be,bg,br,ch,de,dk,es,fr,uk,gr,it,no,pt,ru,se,ua,us}']
}
```

注意：通过 SQL 进行模糊搜索时，MATCH 子句中不应包含除[短语搜索操作符](../Searching/Full_text_matching/Operators.md#Phrase-search-operator)之外的任何全文搜索操作符，且该子句仅应包含您打算匹配的单词。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM mytable WHERE MATCH('someting') OPTION fuzzy=1, layouts='us,ua', distance=2;
```

<!-- request SQL with additional filters -->
附加过滤条件的更复杂模糊搜索查询示例：

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

<!-- request SQL with preserve option -->

```sql
SELECT * FROM mytable WHERE MATCH('hello wrld') OPTION fuzzy=1, preserve=1;
```

<!-- request JSON with preserve option -->

```json
POST /search
{
  "table": "test",
  "query": {
    "bool": {
      "must": [
        {
          "match": {
            "*": "hello wrld"
          }
        }
      ]
    }
  },
  "options": {
    "fuzzy": true,
    "preserve": 1
  }
}
```

<!-- response SQL with preserve option -->

```sql
+------+-------------+
| id   | content     |
+------+-------------+
|    1 | hello wrld  |
|    2 | hello world |
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
    <full-text query>
  },
  "options": {
    "fuzzy": {true|false}
    [,"layouts": ["be","bg","br","ch","de","dk","es","fr","uk","gr","it","no","pt","ru","se","ua","us"]]
    [,"distance": N]
    [,"preserve": {0|1}]
  }
}
```

注意：如果您使用 [query_string](../Searching/Full_text_matching/Basic_usage.md#query_string)，请注意它不支持除[短语搜索操作符](../Searching/Full_text_matching/Operators.md#Phrase-search-operator)之外的全文搜索操作符。查询字符串应仅由您想匹配的单词组成。

### 选项

- `fuzzy`：开启或关闭模糊搜索。
- `distance`：设置匹配的 Levenshtein 距离。默认值为 `2`。
- `preserve`：`0` 或 `1`（默认值：`0`）。设置为 `1` 时，保留搜索结果中没有模糊匹配的单词（例如，“hello wrld”返回“hello wrld”和“hello world”两项）。设置为 `0` 时，仅返回具有成功模糊匹配的单词（例如，“hello wrld”仅返回“hello world”）。对保留短词或可能不存在于 Manticore Search 中的专有名词特别有用。
- `layouts`：键盘布局，用于检测由于键盘布局错位导致的输入错误（例如错误布局下输入“ghbdtn”代替“привет”）。Manticore 会比较不同布局中字符的位置以建议校正。至少需要两个布局才能有效检测错位。默认不使用任何布局。使用空字符串 `''`（SQL）或数组 `[]`（JSON）可关闭此功能。支持的布局包括：
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
  - `ru` - 俄罗斯 JCUKEN 布局
  - `se` - 瑞典 QWERTY 布局
  - `ua` - 乌克兰 JCUKEN 布局
  - `us` - 美国 QWERTY 布局


### 链接

* <a href="https://github.manticoresearch.com/manticoresoftware/manticoresearch?query=fature&filters%5Bcomment%5D%5B%5D=28798446&filters%5Bcommon%5D%5Brepo_id%5D%5B%5D=95614931&sort=&search=keyword-search-fuzzy-layouts">本演示</a> 展示了模糊搜索功能：
  ![模糊搜索示例](fuzzysearch.png){.scale-0.7}
* 关于模糊搜索和自动补全的博客文章 - https://manticoresearch.com/blog/new-fuzzy-search-and-autocomplete/

## CALL QSUGGEST, CALL SUGGEST

两个命令都可以通过 SQL 访问，支持查询本地（普通和实时）以及分布式表。语法如下：
```sql
CALL QSUGGEST(<word or words>, <table name> [,options])
CALL SUGGEST(<word or words>, <table name> [,options])

options: N as option_name[, M as another_option, ...]
```

这些命令为给定单词提供字典中的所有建议。它们仅在启用了[infixing](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)并且设置了[dict=keywords](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)的表上工作。它们返回建议的关键词、建议词与原始关键词之间的莱文斯坦距离，以及建议关键词的文档统计信息。

如果第一个参数包含多个单词，则：
* `CALL QSUGGEST` 仅返回**最后**一个单词的建议，忽略其他单词。
* `CALL SUGGEST` 仅返回**第一个**单词的建议。

这就是两者的唯一不同。支持多种选项以进行自定义：

| 选项 | 说明 | 默认值 |
| - | - | - |
| limit | 返回前 N 个匹配项 | 5 |
| max_edits | 仅保留莱文斯坦距离小于或等于 N 的字典词 | 4 |
| result_stats | 提供找到词的莱文斯坦距离和文档数量 | 1（开启） |
| delta_len | 仅保留长度差异小于 N 的字典词 | 3 |
| max_matches | 保留的匹配数 | 25 |
| reject | 拒绝的词是匹配队列中已经存在的更优匹配不敌的匹配。它们会被放入拒绝队列，如果有一个实际可以进入匹配队列，拒绝队列会重置。此参数定义拒绝队列的大小（为 reject*max(max_matched,limit)）。如果拒绝队列填满，搜索引擎将停止寻找潜在匹配 | 4 |
| result_line | 另一种显示数据的模式，每行分别返回所有建议、距离和文档数 | 0 |
| non_char | 不跳过包含非字母符号的字典词 | 0（跳过此类词） |
| sentence | 返回原始句子，并将最后一个单词替换为匹配词。 | 0（不返回完整句子） |

为了展示其工作方式，我们创建一个表并添加几个文档。

```sql
create table products(title text) min_infix_len='2';
insert into products values (0,'Crossbody Bag with Tassel'), (0,'microfiber sheet set'), (0,'Pet Hair Remover Glove');
```
<!-- example single -->
##### 单词示例
如你所见，拼写错误的单词 "crossb**U**dy" 被更正为 "crossbody"。默认情况下，`CALL SUGGEST/QSUGGEST` 返回：

* `distance` - 莱文斯坦距离，表示为了将给定单词转换为建议词所需的编辑次数
* `docs` - 包含建议词的文档数量

若要禁用这些统计的显示，可以使用选项 `0 as result_stats`。


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
如果第一个参数不是单个单词，而是多个单词，`CALL SUGGEST` 只返回第一个单词的建议。


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
如果第一个参数不是单个单词，而是多个单词，`CALL SUGGEST` 只返回最后一个单词的建议。


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

添加 `1 as sentence` 使得 `CALL QSUGGEST` 返回带有最后一个单词更正后的完整句子。

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
`1 as result_line` 选项改变了建议在输出中的显示方式。不再将每条建议显示为独立行，而是将所有建议、距离和文档数合并显示在一行。以下示例演示该功能：

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

* [这个交互式课程](https://play.manticoresearch.com/didyoumean/) 演示了 `CALL SUGGEST` 在一个小型网页应用中的工作方式。

![CALL SUGGEST 示例](didyoumean.png){.scale-0.5}


<!-- proofread -->

