# 拼写纠正

拼写纠正，也称为：

* 自动纠正
* 文本纠正
* 修正拼写错误
* 容错拼写
* “你的意思是？”

等等，是一种软件功能，用于建议替代词或自动纠正您输入的文本。纠正输入文本的概念可以追溯到20世纪60年代，当时计算机科学家Warren Teitelman（他还发明了“撤销”命令）提出了一种称为D.W.I.M.（“做我意思的事”）的计算哲学。Teitelman认为，计算机不应只接受格式完全正确的指令，而应被编程为识别明显的错误。

第一个提供拼写纠正功能的知名产品是1993年发布的Microsoft Word 6.0。

### 工作原理

拼写纠正可以通过几种方式实现，但重要的是要注意，没有纯粹的程序化方法能以较高质量将您错误输入的“ipone”转换为“iphone”。大多数情况下，系统必须基于某个数据集。该数据集可以是：

* 一个正确拼写单词的词典，词典又可以是：
  * 基于您的真实数据。这里的想法是，词典中由您的数据组成的拼写大部分是正确的，系统尝试找到与输入单词最相似的单词（我们稍后将讨论如何用Manticore实现）。
  * 或者基于与您的数据无关的外部词典。这里可能出现的问题是您的数据和外部词典差异过大：词典中可能缺少某些单词，而您的数据中可能缺少其他单词。
* 不仅基于词典，还基于上下文，例如，“white ber”会被纠正为“white bear”，而“dark ber”会被纠正为“dark beer”。上下文不仅可能是查询中的相邻单词，还可能是您的位置、时间、当前句子的语法（是否将“there”改为“their”）、您的搜索历史，以及几乎任何可能影响您意图的因素。
* 另一种经典方法是使用之前的搜索查询作为拼写纠正的数据集。这在[自动补全](../Searching/Autocomplete.md)功能中使用得更多，但对自动纠正也有意义。其思想是用户大多数拼写是正确的，因此我们可以使用他们的搜索历史中的单词作为事实来源，即使我们在文档中没有这些单词或不使用外部词典。这里也可以实现上下文感知。

Manticore提供了模糊搜索选项以及可用于自动拼写纠正的命令`CALL QSUGGEST`和`CALL SUGGEST`。

## 模糊搜索

模糊搜索功能允许通过考虑搜索查询中的轻微变体或拼写错误，实现更灵活的匹配。它的工作方式类似于普通的`SELECT` SQL语句或`/search` JSON请求，但提供了额外的参数来控制模糊匹配行为。

> 注意：`fuzzy`选项需要[ Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装Buddy。

> 注意：`fuzzy`选项不适用于[多查询](../Searching/Multi-queries.md)。

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
  [, preserve={0|1}]
  [, layouts='{be,bg,br,ch,de,dk,es,fr,uk,gr,it,no,pt,ru,se,ua,us}']
}
```

注意：通过SQL进行模糊搜索时，MATCH子句中不应包含除[短语搜索操作符](../Searching/Full_text_matching/Operators.md#Phrase-search-operator)之外的任何全文操作符，并且应仅包含您希望匹配的单词。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM mytable WHERE MATCH('someting') OPTION fuzzy=1, layouts='us,ua', distance=2;
```

<!-- request SQL with additional filters -->
带有额外过滤条件的更复杂模糊搜索查询示例：

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

注意：如果您使用[query_string](../Searching/Full_text_matching/Basic_usage.md#query_string)，请注意它不支持除[短语搜索操作符](../Searching/Full_text_matching/Operators.md#Phrase-search-operator)之外的全文操作符。查询字符串应仅由您希望匹配的单词组成。

### 选项

- `fuzzy`：开启或关闭模糊搜索。
- `distance`：设置匹配的Levenshtein距离。默认值为`2`。
- `preserve`：`0`或`1`（默认：`0`）。设置为`1`时，保留搜索结果中没有模糊匹配的单词（例如，“hello wrld”返回“hello wrld”和“hello world”）。设置为`0`时，仅返回成功模糊匹配的单词（例如，“hello wrld”仅返回“hello world”）。对于保留可能不存在于Manticore Search中的短词或专有名词特别有用。
- `layouts`：用于检测因键盘布局不匹配导致的输入错误的键盘布局（例如，使用错误布局时输入“ghbdtn”代替“привет”）。Manticore比较不同布局中字符的位置以建议纠正。至少需要2个布局才能有效检测不匹配。默认不使用任何布局。使用空字符串`''`（SQL）或空数组`[]`（JSON）关闭此功能。支持的布局包括：
  - `be` - 比利时AZERTY布局
  - `bg` - 标准保加利亚布局
  - `br` - 巴西QWERTY布局
  - `ch` - 瑞士QWERTZ布局
  - `de` - 德国QWERTZ布局
  - `dk` - 丹麦QWERTY布局
  - `es` - 西班牙QWERTY布局
  - `fr` - 法国AZERTY布局
  - `uk` - 英国QWERTY布局
  - `gr` - 希腊QWERTY布局
  - `it` - 意大利QWERTY布局
  - `no` - 挪威QWERTY布局
  - `pt` - 葡萄牙QWERTY布局
  - `ru` - 俄罗斯JCUKEN布局
  - `se` - 瑞典QWERTY布局
  - `ua` - 乌克兰JCUKEN布局
  - `us` - 美国QWERTY布局


### 链接

* <a href="https://github.manticoresearch.com/manticoresoftware/manticoresearch?query=fature&filters%5Bcomment%5D%5B%5D=28798446&filters%5Bcommon%5D%5Brepo_id%5D%5B%5D=95614931&sort=&search=keyword-search-fuzzy-layouts">此演示</a>展示了模糊搜索功能：
  ![模糊搜索示例](fuzzysearch.png){.scale-0.7}
* 关于模糊搜索和自动补全的博客文章 - https://manticoresearch.com/blog/new-fuzzy-search-and-autocomplete/

## CALL QSUGGEST, CALL SUGGEST

这两个命令都可以通过 SQL 访问，支持查询本地（普通和实时）以及分布式表。语法如下：
```sql
CALL QSUGGEST(<word or words>, <table name> [,options])
CALL SUGGEST(<word or words>, <table name> [,options])

options: N as option_name[, M as another_option, ...]
```

这些命令为给定单词提供字典中的所有建议。它们仅在启用了[infixing](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)且设置了[dict=keywords](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)的表上工作。它们返回建议的关键词、建议关键词与原始关键词之间的 Levenshtein 距离，以及建议关键词的文档统计信息。

如果第一个参数包含多个单词，则：
* `CALL QSUGGEST` 只返回**最后一个**单词的建议，忽略其余部分。
* `CALL SUGGEST` 只返回**第一个**单词的建议。

这就是它们之间唯一的区别。支持多种选项进行自定义：

| 选项 | 描述 | 默认值 |
| - | - | - |
| limit | 返回前 N 个匹配项 | 5 |
| max_edits | 仅保留 Levenshtein 距离小于或等于 N 的字典词 | 4 |
| result_stats | 提供找到词的 Levenshtein 距离和文档计数 | 1（启用） |
| delta_len | 仅保留长度差小于 N 的字典词 | 3 |
| max_matches | 保留的匹配数量 | 25 |
| reject | 被拒绝的词是那些不比匹配队列中已有词更好的匹配。它们被放入一个拒绝队列，如果有词能进入匹配队列，拒绝队列会被重置。此参数定义拒绝队列的大小（为 reject*max(max_matched,limit)）。如果拒绝队列满了，搜索潜在匹配的过程会停止 | 4 |
| result_line | 另一种显示模式，按行返回所有建议、距离和文档数 | 0 |
| non_char | 不跳过包含非字母符号的字典词 | 0（跳过此类词） |
| sentence | 返回原始句子，并将最后一个单词替换为匹配词 | 0（不返回完整句子） |
| force_bigrams | 强制对所有单词长度使用二元组（2字符 n-gram）而非三元组，这可以改善对字符调换错误的匹配 | 0（对长度≥6的单词使用三元组） |

为了展示其工作原理，我们创建一个表并添加几个文档。

```sql
create table products(title text) min_infix_len='2';
insert into products values (0,'Crossbody Bag with Tassel'), (0,'microfiber sheet set'), (0,'Pet Hair Remover Glove');
```
<!-- example single -->
##### 单词示例
如你所见，拼写错误的单词 "crossb**U**dy" 被纠正为 "crossbody"。默认情况下，`CALL SUGGEST/QSUGGEST` 返回：

* `distance` - Levenshtein 距离，表示将给定单词转换为建议词所需的编辑次数
* `docs` - 包含建议词的文档数量

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
##### CALL SUGGEST 只处理第一个单词
如果第一个参数不是单个单词，而是多个单词，则 `CALL SUGGEST` 只返回第一个单词的建议。


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
##### CALL QSUGGEST 只处理最后一个单词
如果第一个参数不是单个单词，而是多个单词，则 `CALL QSUGGEST` 只返回最后一个单词的建议。


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

添加 `1 as sentence` 使 `CALL QSUGGEST` 返回整个句子，并将最后一个单词纠正。

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
`1 as result_line` 选项改变了建议在输出中的显示方式。它不再将每个建议显示在单独的行中，而是将所有建议、距离和文档数显示在一行中。以下示例演示了这一点：

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

##### 使用 force_bigrams 改善调换错误处理
`force_bigrams` 选项可以帮助处理字符调换错误的单词，例如 "ipohne" 与 "iphone"。通过使用二元组而非三元组，算法能更好地处理字符调换。

<!-- intro -->
##### 示例：

<!-- request Example -->

```sql
CALL SUGGEST('ipohne', 'products', 1 as force_bigrams);
```
<!-- response Example -->

```sql
+--------+----------+------+
| suggest| distance | docs |
+--------+----------+------+
| iphone | 2        | 1    |
+--------+----------+------+
```
<!-- end -->

### 演示

* [此交互式课程](https://play.manticoresearch.com/didyoumean/) 展示了 `CALL SUGGEST` 在一个小型网页应用中的工作方式。

![CALL SUGGEST 示例](didyoumean.png){.scale-0.5}


<!-- proofread -->

