# 自动补全

自动补全，也称为词语补全，会在你输入时预测并建议词或短语的结尾。它通常用于：

- 网站的搜索框
- 搜索引擎中的建议
- 应用程序中的文本字段

Manticore 提供了一种高级的自动补全功能，在你输入时会提供类似于知名搜索引擎的建议。这有助于加快搜索速度，让用户更快找到所需内容。

除了基本的自动补全功能外，Manticore 还包括一些高级功能来提升用户体验：

1. **拼写校正（模糊匹配）：** Manticore 的自动补全通过算法识别并修正常见的拼写错误，即使你输入错误，它也能帮助你找到正确的内容。
2. **键盘布局自动检测：** Manticore 能够识别你正在使用的键盘布局。这在多语言环境中非常有用，或者当你意外使用了错误的语言时也能提供帮助。例如，如果你错误地输入了"ghbdtn"，Manticore 会知道你其实想输入的是俄语的"привет"（你好），并为你提供正确的建议。

Manticore 的自动补全功能可以根据不同需求和设置进行调整，使其成为一种灵活的工具，适用于多种应用场景。

![Autocomplete](autocomplete.png)

## CALL AUTOCOMPLETE

> 注意：`CALL AUTOCOMPLETE` 和 `/autocomplete` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法正常运行，请确保 Buddy 已安装。

<!-- example call_autocomplete -->

要在 Manticore 中使用自动补全，可以使用 `CALL AUTOCOMPLETE` SQL 语句或其 JSON 等价的 `/autocomplete` 接口。此功能根据已索引的数据提供词语补全建议。

在使用前，请确保你打算用于自动补全的表启用了 [infixes](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。

**注意：** 表设置中会自动检查 `min_infix_len`，并使用30秒的缓存来提高 `CALL AUTOCOMPLETE` 的性能。在对表进行更改后，第一次使用 `CALL AUTOCOMPLETE` 时可能会有短暂延迟（通常不明显）。只有成功的结果会被缓存，因此如果删除表或禁用 `min_infix_len`，`CALL AUTOCOMPLETE` 可能暂时返回不正确的结果，直到最终显示与 `min_infix_len` 相关的错误。

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

- `layouts`: 一个逗号分隔的键盘布局代码字符串，用于验证和检查拼写校正。可选项：us, ru, ua, se, pt, no, it, gr, uk, fr, es, dk, de, ch, br, bg, be（更多详情请见[这里](../../Searching/Spell_correction.md#Options)）。默认：全启用
- `fuzziness`：`0`、`1` 或 `2`（默认值为 `2`）。用于查找拼写错误的最大 Levenshtein 距离。设置为 `0` 以禁用模糊匹配
- `append`：布尔值（SQL 中为 0/1）。如果为 true（1），则在最后一个词之前加上星号用于前缀扩展（如 `*word`）
- `expansion_len`：最后一个词扩展的字符数。默认值为 `10`

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

<!-- end -->

#### 链接
* [这个演示](https://github.manticoresearch.com/manticoresoftware/manticoresearch) 展示了自动补全功能:
  ![Autocomplete example](autocomplete_github_demo.png){.scale-0.7}
* 关于模糊搜索和自动补全的博客文章 - https://manticoresearch.com/blog/new-fuzzy-search-and-autocomplete/

## 替代的自动补全方法

虽然 `CALL AUTOCOMPLETE` 是大多数使用场景中的推荐方法，Manticore 也支持其他可控且可定制的实现自动补全的方式：

##### 自动补全句子
要自动补全句子，可以使用 [中缀搜索](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。你可以通过提供字段的开头并：

- 使用 [全文通配符操作符](../Searching/Full_text_matching/Operators.md) `*` 匹配任意字符
- 可选地使用 `^` 从字段的开头开始
- 可选地使用 `""` 进行短语匹配
- 并使用 [结果高亮](../Searching/Highlighting.md)

关于这点有一篇 [博客文章](https://manticoresearch.com/2020/03/31/simple-autocomplete-with-manticore/) 和一个 [互动课程](https://play.manticoresearch.com/simpleautocomplete/)。一个简单的例子是：

- 假设你有一个文档：`My cat loves my dog. The cat (Felis catus) is a domestic species of small carnivorous mammal.`
- 然后你可以使用 `^`，`""` 和 `*`，让用户输入时，你可以做如下查询：`^"m*"`, `^"my *"`, `^"my c*"`, `^"my ca*"` 等等
- 它将找到文档，并且如果你也做了 [高亮](../Searching/Highlighting.md)，你将得到类似 `<b>My cat</b> loves my dog. The cat ( ...` 的结果

##### 自动补全单词
在某些情况下，你可能只需要自动补全一个单词或几个单词。在这种情况下，你可以使用 `CALL KEYWORDS`。

### CALL KEYWORDS
`CALL KEYWORDS` 可通过 SQL 接口使用，它提供了一种检查关键词如何被分词或获取特定关键词的分词形式的方式。如果表启用了 [中缀](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)，它可以快速找到给定关键词的可能结尾，因此适用于自动补全功能。

这是通用中缀搜索的一个很好的替代方案，因为它只需要表的词典，而不需要文档本身。

### 通用语法
<!-- example keywords -->
```sql
CALL KEYWORDS(text, table [, options])
```
`CALL KEYWORDS` 语句将文本划分为关键词。它返回关键词的分词和标准化形式，并在需要时显示关键词的统计数据。此外，当表启用了 [词法分析器](../Creating_a_table/NLP_and_tokenization/Morphology.md) 时，它还提供每个关键词在查询中的位置以及所有分词关键词的形式。

| 参数                   | 描述                                                         |
| ---------------------- | ------------------------------------------------------------ |
| text                   | 要拆分成关键词的文本                                         |
| table                  | 从中获取文本处理设置的表的名称                               |
| 0/1 as stats           | 显示关键词统计数据，默认值为 0                               |
| 0/1 as fold_wildcards  | 折叠通配符，默认值为 0                                       |
| 0/1 as fold_lemmas     | 折叠形态词根，默认值为 0                                     |
| 0/1 as fold_blended    | 折叠混合词，默认值为 0                                       |
| N as expansion_limit   | 覆盖服务器配置中定义的 [expansion_limit](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit)，默认值为 0（使用配置中的值） |
| docs/hits as sort_mode | 按 "docs" 或 "hits" 排序输出结果。默认不排序                 |

这些例子展示了它如何工作，假设用户试图获取 "my cat ..." 的自动补全。因此在应用程序端，你只需为用户建议每个新单词的 "normalized" 列中的结尾。通常使用 `'hits' as sort_mode` 或 `'docs' as sort_mode` 按点击数或文档数排序是有意义的。

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

有一个很好的技巧可以改进上述算法——使用 [bigram_index](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)。当为表启用它时，索引中的不仅仅是单个单词，而是相邻的每一对单词，它们作为单独的标记进行索引。

这不仅允许预测当前单词的结尾，还可以预测下一个单词，对于自动补全的目的尤其有益。

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

`CALL KEYWORDS` 支持分布式表，因此无论数据集有多大，您都可以从中受益。
<!-- proofread -->
