# 拼写纠正

拼写纠正，也称为：

* 自动更正
* 文字纠正
* 修正拼写错误
* 容错拼写
* “你是想说……吗？”

等等，是一种软件功能，建议替代文本或自动更正您输入的文本。纠正输入文本的概念可以追溯到20世纪60年代，当时计算机科学家沃伦·泰特尔曼（Warren Teitelman），他还发明了“撤销”命令，提出了一种称为D.W.I.M.（即“做我意思”的缩写）的计算哲学。泰特尔曼认为，电脑不应仅仅接受完美格式化的指令，而应被编程来识别明显的错误。

第一个知名提供拼写纠正功能的产品是1993年发布的Microsoft Word 6.0。

### 工作原理

拼写纠正可以通过几种方式实现，但重要的是要注意，没有一种纯程序化的方法能高质量地将您错误输入的“ipone”转换为“iphone”。通常，这基于系统依赖的数据集。数据集可以是：

* 一个正确拼写单词的字典，字典又可以是：
  * 基于您的真实数据。这里的想法是，您数据中的拼写在大多数情况下是正确的，系统会尝试找到与输入词最相似的词（我们稍后将讨论如何使用Manticore实现）。
  * 或者基于与您数据无关的外部字典。可能出现的问题是您的数据和外部字典差异过大：一些词在字典中缺失，而另一些则在您的数据中缺失。
* 不仅基于字典，还考虑上下文，比如“white ber”会被纠正为“white bear”，而“dark ber”会被纠正为“dark beer”。上下文不仅可能是查询中的邻近词，还可以是您的所在地、时间、当前句子的语法（例如是否将“there”改为“their”）、您的搜索历史，乃至任何可能影响您意图的因素。
* 另一种经典做法是将以前的搜索查询用作拼写纠正的数据集。这在[自动完成](../Searching/Autocomplete.md)功能中被大量使用，也适用于自动纠正。其想法是用户大多数情况下拼写正确，因此我们可以将他们搜索历史中的单词作为事实来源，即使这些词不存在于我们的文档中或外部字典中。这里也可以实现上下文感知。

Manticore提供了模糊搜索选项以及`CALL QSUGGEST`和`CALL SUGGEST`命令，可用于自动拼写纠正。

## 模糊搜索

模糊搜索功能允许更灵活的匹配，考虑搜索查询中的轻微变化或拼写错误。它的工作方式类似于普通的`SELECT` SQL语句或`/search` JSON请求，但提供了额外参数来控制模糊匹配行为。

> 注意：`fuzzy`选项需要[安装Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确认Buddy已安装。

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

注意：使用SQL进行模糊搜索时，MATCH子句不应包含除[短语搜索操作符](../Searching/Full_text_matching/Operators.md#Phrase-search-operator)外的任何全文操作符，且只应包含您想匹配的词语。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM mytable WHERE MATCH('someting') OPTION fuzzy=1, layouts='us,ua', distance=2;
```

<!-- request SQL with additional filters -->
包含额外筛选条件的更复杂模糊搜索查询示例：

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
    [,\"preserve\": {0|1}]
  }
}
```

注意：如果您使用[query_string](../Searching/Full_text_matching/Basic_usage.md#query_string)，请注意它不支持除[短语搜索操作符](../Searching/Full_text_matching/Operators.md#Phrase-search-operator)之外的全文操作符。查询字符串应仅包含您想匹配的词语。

### 选项

- `fuzzy`：打开或关闭模糊搜索。
- `distance`：设置匹配的Levenshtein距离。默认值为`2`。
- `preserve`：`0`或`1`（默认：`0`）。设置为`1`时，搜索结果保留没有模糊匹配的词（例如，“hello wrld”返回“hello wrld”和“hello world”）。设置为`0`时，仅返回成功模糊匹配的词（例如，“hello wrld”仅返回“hello world”）。这对保留短词或可能不存在于Manticore Search中的专有名词尤为有用。
- `layouts`：键盘布局，用于检测因键盘布局错误导致的输入错误（例如使用错误布局打出“ghbdtn”而非“привет”）。Manticore会比较不同布局间字符的位置以提供纠正建议。至少需使用两种布局来有效检测错误。默认不使用任何布局。使用空字符串`''`（SQL）或空数组`[]`（JSON）关闭此功能。支持的布局包括：
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

两个命令都可以通过 SQL 访问，并支持查询本地（普通和实时）及分布式表。语法如下：
```sql
CALL QSUGGEST(<word or words>, <table name> [,options])
CALL SUGGEST(<word or words>, <table name> [,options])

options: N as option_name[, M as another_option, ...]
```

这些命令为给定单词提供词典中的所有建议。它们仅适用于启用了[infixing](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)并且设置了[dict=keywords](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)的表。它们返回建议的关键词、建议词与原始词之间的 Levenshtein 距离以及建议关键词的文档统计信息。

如果第一个参数包含多个词，则：
* `CALL QSUGGEST` 仅返回对**最后一个**词的建议，忽略其余部分。
* `CALL SUGGEST` 仅返回对**第一个**词的建议。

这就是它们之间的唯一区别。支持多个选项以供定制：

| 选项 | 描述 | 默认值 |
| - | - | - |
| limit | 返回前 N 个匹配结果 | 5 |
| max_edits | 仅保留 Levenshtein 距离小于等于 N 的词典词 | 4 |
| result_stats | 提供找到的词的 Levenshtein 距离和文档计数 | 1（启用） |
| delta_len | 仅保留长度差小于 N 的词典词 | 3 |
| max_matches | 保留的匹配数量 | 25 |
| reject | 拒绝的词是相较于已在匹配队列中的词没有更优表现的匹配词。它们被放入一个拒绝队列，当有一个词能进入匹配队列时该队列重置。此参数定义拒绝队列的大小（为 reject*max(max_matched,limit)）。如果拒绝队列已满，引擎将停止寻找潜在匹配 | 4 |
| result_line | 另一种模式，按行返回所有建议、距离和文档数 | 0 |
| non_char | 不跳过包含非字母符号的词典词 | 0（跳过这类词） |
| sentence | 返回原句但最后一个词被匹配词替换 | 0（不返回完整句子） |

为了演示其工作原理，创建一个表并添加一些文档。

```sql
create table products(title text) min_infix_len='2';
insert into products values (0,'Crossbody Bag with Tassel'), (0,'microfiber sheet set'), (0,'Pet Hair Remover Glove');
```
<!-- example single -->
##### 单词示例
如你所见，拼写错误的单词 "crossb**U**dy" 被纠正为 "crossbody"。默认情况下，`CALL SUGGEST/QSUGGEST` 返回：

* `distance` - Levenshtein 距离，表示将给定单词转换为建议词所需的编辑步骤数
* `docs` - 包含建议词的文档数

要禁用这些统计的显示，可以使用选项 `0 as result_stats`。


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
##### CALL SUGGEST 只处理第一个词
如果第一个参数不是单词而是多个，`CALL SUGGEST` 仅返回第一个词的建议。


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
##### CALL QSUGGEST 只处理最后一个词
如果第一个参数不是单词而是多个，`CALL QSUGGEST` 仅返回最后一个词的建议。


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

添加 `1 as sentence` 会让 `CALL QSUGGEST` 返回整个句子，并将最后一个词更正。

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
选项 `1 as result_line` 改变了建议在输出中的显示方式。它不再把每个建议分成一行，而是在一行内显示所有建议、距离和文档计数。下面是示例演示：

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

* [此互动课程](https://play.manticoresearch.com/didyoumean/) 展示了 `CALL SUGGEST` 在小型网页应用中的工作方式。

![CALL SUGGEST 示例](didyoumean.png){.scale-0.5}


<!-- proofread -->

