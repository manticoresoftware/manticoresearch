# 自动补全

自动补全，或称为单词补全，是在你输入时预测并建议单词或短语的结尾。它通常用于：
- 网站上的搜索框
- 搜索引擎中的建议
- 应用程序中的文本字段

Manticore 提供了一个高级自动补全功能，能够在输入时即时给出建议，类似于知名搜索引擎的功能。这有助于加快搜索速度，让用户更快找到所需内容。

除了基本的自动补全功能外，Manticore 还包含高级功能，以提升用户体验：

1. **拼写纠正（模糊匹配）：** Manticore 的自动补全通过使用识别并修正常见错误的算法，帮助纠正拼写错误。这意味着即使你输入错误，仍然可以找到你想找的内容。
2. **键盘布局自动检测：** Manticore 能自动识别你使用的键盘布局。这在多语言环境中或你意外使用了错误语言时非常有用。例如，如果你误输“ghbdtn”，Manticore 会知道你是想输入俄语的“привет”（你好），并建议正确的单词。

Manticore 的自动补全可以根据不同需求和设置定制，成为多种应用的灵活工具。

![Autocomplete](autocomplete.png)

## 调用自动补全（CALL AUTOCOMPLETE）

> 注意：`CALL AUTOCOMPLETE` 和 `/autocomplete` 需要安装 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确认 Buddy 已安装。

<!-- example call_autocomplete -->
在 Manticore 中使用自动补全，请使用 `CALL AUTOCOMPLETE` SQL 语句或其 JSON 等价 `/autocomplete`。该功能基于你的索引数据提供单词补全建议。

在继续之前，请确保你打算用于自动补全的表已经开启了 [infixes](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。

**注意：** 表设置中自动检查 `min_infix_len`，并使用30秒缓存以提高 `CALL AUTOCOMPLETE` 的性能。修改表后，首次调用 `CALL AUTOCOMPLETE` 可能会有短暂延迟（通常不明显）。仅缓存成功结果，所以如果你删除表或禁用 `min_infix_len`，`CALL AUTOCOMPLETE` 可能暂时返回错误结果，直到最终显示与 `min_infix_len` 相关的错误。

### 通用语法

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
- `layouts`：逗号分隔的键盘布局代码字符串，用于检测由键盘布局不匹配导致的输入错误（例如错误布局下输入“ghbdtn”而非“привет”）。Manticore 会比较不同布局的字符位置以建议更正。要求至少两个布局才能有效检测不匹配。可用布局选项：us, ru, ua, se, pt, no, it, gr, uk, fr, es, dk, de, ch, br, bg, be（详细说明见 [这里](../Searching/Spell_correction.md#Options)）。默认值：无
- `fuzziness`：`0`，`1`，或`2`（默认：`2`）。用于查找拼写错误的最大 Levenshtein 距离。设置为 `0` 可禁用模糊匹配
- `preserve`：`0` 或 `1`（默认：`0`）。设置为 `1` 时，保留搜索结果中无模糊匹配的单词（例如，“hello wrld” 返回“hello wrld”和“hello world”）。设置为 `0` 时，只返回有成功模糊匹配的单词（例如，“hello wrld” 仅返回“hello world”）。特别适用于保留短单词或可能不存在于 Manticore Search 的专有名词
- `prepend`：布尔值（SQL中为0/1）。如果为真(1)，则在最后一个单词前添加星号以进行前缀扩展（例如 `*word`）
- `append`：布尔值（SQL中为0/1）。如果为真(1)，则在最后一个单词后添加星号以进行后缀扩展（例如 `word*`）
- `expansion_len`：扩展最后一个单词的字符数。默认值：`10`
- `force_bigrams`：布尔值（SQL中0/1）。强制对所有单词长度使用二元组（2字符 n-gram）而非三元组，以改善针对字符转置错误的匹配。默认值：`0`（单词长度≥6时使用三元组）

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

<!-- example force_bigrams option -->
##### 使用 force_bigrams 更好地处理字符转置错误
`force_bigrams` 选项有助于处理字符转置错误的单词，例如“ipohne”与“iphone”。通过使用二元组而非三元组，算法能够更好地处理字符位置颠倒的问题。

<!-- request SQL -->
```sql
mysql> CALL AUTOCOMPLETE('ipohne', 'products', 1 as force_bigrams);
```

<!-- response SQL -->
```
+--------+
| query  |
+--------+
| iphone |
+--------+
```

<!-- request JSON -->
```json
POST /autocomplete
{
	"table":"products",
	"query":"ipohne",
	"options": {
		"force_bigrams": 1
	}
}
```

<!-- response JSON -->
```json
[
  {
    "total": 1,
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
        "query": "iphone"
      }
    ]
  }
]
```

<!-- end -->

#### 链接
* [此演示](https://github.manticoresearch.com/manticoresoftware/manticoresearch) 演示了自动补全功能：
  ![Autocomplete example](autocomplete_github_demo.png){.scale-0.7}
* 关于模糊搜索和自动补全的博客文章 - https://manticoresearch.com/blog/new-fuzzy-search-and-autocomplete/

## 替代自动补全方法

尽管 `CALL AUTOCOMPLETE` 是大多数使用场景推荐的方法，Manticore 也支持其他可控且可自定义的方法来实现自动补全功能：

##### 补全句子
要补全一句话，可以使用 [infix 搜索](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。你可以通过给出文档字段的开头并：
* 使用[全文通配符操作符](../Searching/Full_text_matching/Operators.md) `*` 来匹配任意字符
* 可选地使用 `^` 从字段开头开始匹配
* 可选地使用 `""` 进行短语匹配
* 并使用[结果高亮](../Searching/Highlighting.md)

有一篇关于它的[博客文章](https://manticoresearch.com/blog/simple-autocomplete-with-manticore/)以及一个[交互式课程](https://play.manticoresearch.com/simpleautocomplete/)。一个快速示例如下：
* 假设你有这样一段文档：`My cat loves my dog. The cat (Felis catus) is a domestic species of small carnivorous mammal.`
* 然后你可以使用 `^`、`""` 和 `*`，这样当用户输入时，你可以发出类似 `^"m*"`、`^"my *"`、`^"my c*"`、`^"my ca*"` 等查询
* 它将找到该文档，如果你还做了[高亮](../Searching/Highlighting.md)，你将得到类似：`<b>My cat</b> loves my dog. The cat ( ...`

##### 自动补全单词
在某些情况下，你只需要自动补全一个单词或几个单词。在这种情况下，你可以使用 `CALL KEYWORDS`。

### CALL KEYWORDS
`CALL KEYWORDS` 可通过 SQL 接口使用，提供了一种检查关键字如何被分词，或获取特定关键字的分词形式的方法。如果表启用了[中缀](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)，它可以快速找到给定关键字的可能结尾，适合用于自动补全功能。

这比通用的中缀搜索是一种很好的替代方案，因为它性能更高，只需使用表的字典，而不需要文档本身。

### 通用语法
<!-- example keywords -->
```sql
CALL KEYWORDS(text, table [, options])
```
`CALL KEYWORDS` 语句将文本划分为关键字。它返回关键字的分词和规范化形式，如果需要，还可以返回关键字统计信息。此外，当表启用了[词形还原器](../Creating_a_table/NLP_and_tokenization/Morphology.md)时，还会提供查询中每个关键字的位置及所有分词形式。

| 参数 | 说明 |
| - | - |
| text | 需要拆分的文本 |
| table | 用以获取文本处理设置的表名称 |
| 0/1 as stats | 是否显示关键字统计，默认是 0 |
| 0/1 as fold_wildcards | 是否折叠通配符，默认是 0 |
| 0/1 as fold_lemmas | 是否折叠形态词形，默认是 0 |
| 0/1 as fold_blended | 是否折叠混合词，默认是 0 |
| N as expansion_limit | 覆盖服务器配置中定义的[expansion_limit](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit)，默认是 0（使用配置值） |
| docs/hits as sort_mode | 按 ‘docs’ 或 ‘hits’ 排序输出结果，默认不排序 |
| jieba_mode | 查询的结巴分词模式，详见[jieba_mode](../Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode) |

以下示例展示了当用户尝试获取“my cat ...”的自动补全时它是如何工作的。因此，在应用端你只需针对每个新词建议“normalized”列中的结尾。通常使用 `'hits' as sort_mode` 或 `'docs' as sort_mode` 来排序会更合理。

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
这里有一个很好的技巧可以改进上述算法——使用[bigram_index](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)。当你为表启用它时，索引中不仅包含单个词，还包含每对相邻词作为独立的词条。

这不仅能预测当前单词的结尾，还能预测下一个单词，对自动补全目标特别有益。

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

`CALL KEYWORDS` 支持分布式表，无论你的数据集多大，都能利用它的优势。 
<!-- proofread -->

