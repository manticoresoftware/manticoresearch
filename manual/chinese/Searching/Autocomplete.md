# 自动补全

自动补全，或称词语补全，能够在你输入时预测并建议单词或短语的结尾部分。它常用于：
- 网站的搜索框
- 搜索引擎中的建议
- 应用程序中的文本字段

Manticore 提供了先进的自动补全功能，能够在输入时给出建议，类似于知名搜索引擎中的功能。这有助于加快搜索速度，让用户更快找到所需内容。

除了基本的自动补全功能外，Manticore 还包含一些高级功能，以提升用户体验：

1. **拼写纠错（模糊匹配）：** Manticore 的自动补全通过算法识别并修正常见错误，帮助纠正拼写错误。也就是说，即使输入错误，依然可以找到想要的内容。
2. **键盘布局自动检测：** Manticore 可以识别你使用的键盘布局。这在多语言环境或者你误用了错误语言输入时非常有用。例如，如果你错误地输入了"ghbdtn"，Manticore 会识别你想输入的是俄语的 "привет"（你好），并建议正确的词。

Manticore 的自动补全可以根据不同需求和设置进行定制，是许多应用中的灵活工具。

![自动补全](autocomplete.png)

## 调用自动补全 CALL AUTOCOMPLETE

> 注意：`CALL AUTOCOMPLETE` 和 `/autocomplete` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确保 Buddy 已安装。

<!-- example call_autocomplete -->
要在 Manticore 中使用自动补全，请使用 `CALL AUTOCOMPLETE` SQL 语句或其 JSON 等效命令 `/autocomplete`。此功能根据你的索引数据提供单词补全建议。

使用前，请确保你用于自动补全的表已经启用了 [中缀搜索](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。

**注意：** 对表设置中的 `min_infix_len` 有自动检查，且使用 30 秒缓存以提升 `CALL AUTOCOMPLETE` 的性能。更改表后，首次调用 `CALL AUTOCOMPLETE` 可能会有短暂延时（通常不易察觉）。只有成功结果会被缓存，如果删除表或禁用 `min_infix_len`，`CALL AUTOCOMPLETE` 可能暂时返回错误结果，直到最终显示与 `min_infix_len` 相关的错误。

### 一般语法

#### SQL
```sql
CALL AUTOCOMPLETE('query_beginning', 'table', [...options]);
```

#### JSON
```json
POST /autocomplete
{
	"table":"table_name",
	"query":"query_beginning"
	[,"options": {<autocomplete options>}]
}
```

#### 选项
- `layouts`：逗号分隔的键盘布局代码字符串，用于检测因键盘布局错误导致的输入错误（例如，使用错误布局时输入 "ghbdtn" 代替 "привет"）。Manticore 会比较不同布局中字符的位置以提供修正建议。需要至少两个布局以有效检测布局不匹配。可用选项：us、ru、ua、se、pt、no、it、gr、uk、fr、es、dk、de、ch、br、bg、be（详情见[这里](../Searching/Spell_correction.md#Options)）。默认：无
- `fuzziness`：取值 `0`、`1` 或 `2`（默认：`2`）。用于查找拼写错误的最大 Levenshtein 距离。设为 `0` 可禁用模糊匹配
- `preserve`：取值 `0` 或 `1`（默认：`0`）。若设为 `1`，搜索结果中保留没有模糊匹配的词（例如，"hello wrld" 会返回 "hello wrld" 和 "hello world"）；设为 `0` 则只返回模糊匹配成功的词（例如，"hello wrld" 只返回 "hello world"）。在保留短词或专有名词（可能不存在于 Manticore Search 中）时特别有用
- `prepend`：布尔值（SQL 中用 0/1 表示）。若为 true(1)，则在最后一个词前添加星号实现前缀扩展（例如，`*word`）
- `append`：布尔值（SQL 中用 0/1 表示）。若为 true(1)，则在最后一个词后添加星号实现后缀扩展（例如，`word*`）
- `expansion_len`：最后一个词的扩展字符数。默认值为 `10`

<!-- request SQL -->

```sql
mysql> CALL AUTOCOMPLETE('hello', 'comment');
+------------+
| query      |
+------------+
| hello      |
| helio      |
| hell       |
| shell      |
| nushell    |
| powershell |
| well       |
| help       |
+------------+
```

<!-- request SQL with no fuzzy search -->

```sql
mysql> CALL AUTOCOMPLETE('hello', 'comment', 0 as fuzziness);
+-------+
| query |
+-------+
| hello |
+-------+
```

<!-- request JSON -->

```json
POST /autocomplete
{
	"table":"comment",
	"query":"hello"
}
```

<!-- response JSON -->
```json
[
  {
    "total": 8,
    "error": "",
    "warning": "",
    "columns": [
      {
        "query": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "query": "hello"
      },
      {
        "query": "helio"
      },
      {
        "query": "hell"
      },
      {
        "query": "shell"
      },
      {
        "query": "nushell"
      },
      {
        "query": "powershell"
      },
      {
        "query": "well"
      },
      {
        "query": "help"
      }
    ]
  }
]
```

<!-- request SQL with preserve option -->

```sql
mysql> CALL AUTOCOMPLETE('hello wrld', 'comment', 1 as preserve);
+------------+
| query      |
+------------+
| hello wrld |
| hello world|
+------------+
```

<!-- request JSON with preserve option -->

```json
POST /autocomplete
{
	"table":"comment",
	"query":"hello wrld",
	"options": {
		"preserve": 1
	}
}
```

<!-- response JSON with preserve option -->
```json
[
  {
    "total": 2,
    "error": "",
    "warning": "",
    "columns": [
      {
        "query": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "query": "hello wrld"
      },
      {
        "query": "hello world"
      }
    ]
  }
]
```

<!-- end -->

#### 链接
* [此演示](https://github.manticoresearch.com/manticoresoftware/manticoresearch) 展示了自动补全功能：
  ![自动补全示例](autocomplete_github_demo.png){.scale-0.7}
* 关于模糊搜索和自动补全的博客文章 - https://manticoresearch.com/blog/new-fuzzy-search-and-autocomplete/

## 替代的自动补全方法

虽然 `CALL AUTOCOMPLETE` 是大多数使用场景下推荐的方法，Manticore 也支持其他灵活且可定制的方法来实现自动补全功能：

##### 自动补全句子
要自动补全文本句子，可以使用 [中缀搜索](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。你可以通过提供文档字段的开始部分来找到结尾，并且：
* 使用 [全文通配符操作符](../Searching/Full_text_matching/Operators.md) `*` 匹配任意字符
* 可选地使用 `^` 来从字段开头开始匹配
* 可选地使用 `""` 进行词组匹配
* 结合使用 [结果高亮](../Searching/Highlighting.md)

我们博客中有[相关介绍文章](https://manticoresearch.com/blog/simple-autocomplete-with-manticore/)以及[交互式课程](https://play.manticoresearch.com/simpleautocomplete/)。一个简单示例是：
* 假设你有一条文档：`My cat loves my dog. The cat (Felis catus) is a domestic species of small carnivorous mammal.`
* 然后你可以使用 `^`、`""` 和 `*`，根据用户输入构造如下查询：`^"m*"`, `^"my *"`, `^"my c*"`, `^"my ca*"` 等
* 它会找到文档，如果同时使用[高亮功能](../Searching/Highlighting.md)，结果可能类似：`<b>My cat</b> loves my dog. The cat ( ...`

##### 自动补全单词
在某些情况下，您只需要自动完成一个单词或几个单词。此时，您可以使用 `CALL KEYWORDS`。

### CALL KEYWORDS
`CALL KEYWORDS` 可通过 SQL 接口使用，提供了一种检查关键词如何被分词或获取特定关键词的分词形式的方法。如果表启用了[infixes](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)，它允许您快速找到给定关键词的可能结尾，适合自动完成功能。

这是通用中缀搜索的一个很好的替代方案，因为它提供了更高的性能，因为它只需要表的字典，而不需要实际的文档。

### 通用语法
<!-- example keywords -->
```sql
CALL KEYWORDS(text, table [, options])
```
`CALL KEYWORDS` 语句将文本拆分为关键词。它返回关键词的分词和规范化形式，并且如果需要，还可以返回关键词统计信息。此外，当表启用[词形还原器](../Creating_a_table/NLP_and_tokenization/Morphology.md)时，它还提供查询中每个关键词的位置及分词关键词的所有形式。

| 参数 | 说明 |
| - | - |
| text | 要拆分成关键词的文本 |
| table | 用于获取文本处理设置的表名 |
| 0/1 as stats | 显示关键词统计，默认值为0 |
| 0/1 as fold_wildcards | 折叠通配符，默认值为0 |
| 0/1 as fold_lemmas | 折叠词形还原词，默认值为0 |
| 0/1 as fold_blended | 折叠混合词，默认值为0 |
| N as expansion_limit | 覆盖服务器配置中定义的[expansion_limit](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit)，默认值为0（使用配置中的值） |
| docs/hits as sort_mode | 通过“docs”或“hits”排序输出结果。默认不进行排序。 |
| jieba_mode | 查询的结巴分词模式。详情见[jieba_mode](../Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode) |

以下示例演示了如果用户试图为“my cat ...”做自动补全时的工作方式。因此，在应用程序端，您只需要为用户建议每个新词的“normalized”列中的结尾。通常，使用 `'hits' as sort_mode` 或 `'docs' as sort_mode` 按命中次数或文档数排序是有意义的。

<!-- intro -->
##### 示例：

<!-- request Examples -->

```sql
MySQL [(none)]> CALL KEYWORDS('m*', 't', 1 as stats);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | m*        | my         | 1    | 2    |
| 1    | m*        | mammal     | 1    | 1    |
+------+-----------+------------+------+------+

MySQL [(none)]> CALL KEYWORDS('my*', 't', 1 as stats);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | my*       | my         | 1    | 2    |
+------+-----------+------------+------+------+

MySQL [(none)]> CALL KEYWORDS('c*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+-------------+------+------+
| qpos | tokenized | normalized  | docs | hits |
+------+-----------+-------------+------+------+
| 1    | c*        | cat         | 1    | 2    |
| 1    | c*        | carnivorous | 1    | 1    |
| 1    | c*        | catus       | 1    | 1    |
+------+-----------+-------------+------+------+

MySQL [(none)]> CALL KEYWORDS('ca*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+-------------+------+------+
| qpos | tokenized | normalized  | docs | hits |
+------+-----------+-------------+------+------+
| 1    | ca*       | cat         | 1    | 2    |
| 1    | ca*       | carnivorous | 1    | 1    |
| 1    | ca*       | catus       | 1    | 1    |
+------+-----------+-------------+------+------+

MySQL [(none)]> CALL KEYWORDS('cat*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | cat*      | cat        | 1    | 2    |
| 1    | cat*      | catus      | 1    | 1    |
+------+-----------+------------+------+------+
```
<!-- end -->
<!-- example bigram -->
有一个不错的技巧可以改进上述算法 —— 使用[bigram_index](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)。当您为表启用它时，获得的不仅仅是单个词，而是相邻成对词被索引为独立的标记。

这不仅能预测当前词的结尾，还能预测下一个词，对于自动完成的目的尤其有利。

<!-- intro -->
##### 示例：

<!-- request Examples -->

```sql
MySQL [(none)]> CALL KEYWORDS('m*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | m*        | my         | 1    | 2    |
| 1    | m*        | mammal     | 1    | 1    |
| 1    | m*        | my cat     | 1    | 1    |
| 1    | m*        | my dog     | 1    | 1    |
+------+-----------+------------+------+------+

MySQL [(none)]> CALL KEYWORDS('my*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | my*       | my         | 1    | 2    |
| 1    | my*       | my cat     | 1    | 1    |
| 1    | my*       | my dog     | 1    | 1    |
+------+-----------+------------+------+------+

MySQL [(none)]> CALL KEYWORDS('c*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+--------------------+------+------+
| qpos | tokenized | normalized         | docs | hits |
+------+-----------+--------------------+------+------+
| 1    | c*        | cat                | 1    | 2    |
| 1    | c*        | carnivorous        | 1    | 1    |
| 1    | c*        | carnivorous mammal | 1    | 1    |
| 1    | c*        | cat felis          | 1    | 1    |
| 1    | c*        | cat loves          | 1    | 1    |
| 1    | c*        | catus              | 1    | 1    |
| 1    | c*        | catus is           | 1    | 1    |
+------+-----------+--------------------+------+------+

MySQL [(none)]> CALL KEYWORDS('ca*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+--------------------+------+------+
| qpos | tokenized | normalized         | docs | hits |
+------+-----------+--------------------+------+------+
| 1    | ca*       | cat                | 1    | 2    |
| 1    | ca*       | carnivorous        | 1    | 1    |
| 1    | ca*       | carnivorous mammal | 1    | 1    |
| 1    | ca*       | cat felis          | 1    | 1    |
| 1    | ca*       | cat loves          | 1    | 1    |
| 1    | ca*       | catus              | 1    | 1    |
| 1    | ca*       | catus is           | 1    | 1    |
+------+-----------+--------------------+------+------+

MySQL [(none)]> CALL KEYWORDS('cat*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | cat*      | cat        | 1    | 2    |
| 1    | cat*      | cat felis  | 1    | 1    |
| 1    | cat*      | cat loves  | 1    | 1    |
| 1    | cat*      | catus      | 1    | 1    |
| 1    | cat*      | catus is   | 1    | 1    |
+------+-----------+------------+------+------+
```
<!-- end -->

`CALL KEYWORDS` 支持分布式表，因此无论您的数据集有多大，都可以使用它获得好处。
<!-- proofread -->

