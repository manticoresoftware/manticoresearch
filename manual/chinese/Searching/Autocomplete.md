# 自动补全

自动补全或单词补全，预测并建议您输入时单词或短语的结尾。它通常用于：
- 网站上的搜索框
- 搜索引擎中的建议
- 应用中的文本字段

Manticore 提供了一项高级自动补全功能，可以在您输入时提供类似知名搜索引擎的建议。这有助于加快搜索速度，并让用户更快找到他们需要的东西。

除了基本的自动补全功能外，Manticore 还包含增强用户体验的高级功能：

1. **拼写校正（模糊性）：** Manticore 的自动补全通过使用识别和修正常见错误的算法来帮助纠正拼写错误。这意味着，即使您输入错误，您仍然可以找到您所寻找的内容。
2. **键盘布局自动检测：** Manticore 可以识别您使用的键盘布局。这在多种语言使用的地方特别有用，或者如果您不小心输入了错误的语言。例如，如果您错误地输入“ghbdtn”，Manticore 知道您想说的是“привет”（俄语中的hello），并建议正确的单词。

Manticore 的自动补全可以根据不同的需求和设置进行定制，使其成为多种应用程序的灵活工具。

![自动补全](autocomplete.png)

## 调用自动补全

> 注意：`CALL AUTOCOMPLETE` 和 `/autocomplete` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

<!-- example call_autocomplete -->
要在 Manticore 中使用自动补全，请使用 `CALL AUTOCOMPLETE` SQL 语句或其 JSON 等价物 `/autocomplete`。此功能根据您的索引数据提供单词补全建议。

在继续之前，请确保您打算用于自动补全的表已启用 [infixes](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。

**注意：** 表设置中会自动检查 `min_infix_len`，该检查使用 30 秒缓存来提高 `CALL AUTOCOMPLETE` 的性能。对表进行更改后，在首次使用 `CALL AUTOCOMPLETE` 时可能会有短暂的延迟（尽管通常不明显）。只有成功的结果会被缓存，因此如果您删除表或禁用 `min_infix_len`，`CALL AUTOCOMPLETE` 可能会暂时返回不正确的结果，直到最终开始显示与 `min_infix_len` 相关的错误。

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
- `layouts`: 以逗号分隔的键盘布局代码字符串，用于验证和检查拼写校正。可用选项：us, ru, ua, se, pt, no, it, gr, uk, fr, es, dk, de, ch, br, bg, be（更多详细信息 [在这里](../../Searching/Spell_correction.md#Options)）。默认：所有启用
- `fuzziness`: `0`，`1` 或 `2`（默认：`2`）。查找拼写错误的最大 Levenshtein 距离。设置为 `0` 以禁用模糊匹配
- `prepend`: 布尔值（在 SQL 中为 0/1）。如果为 true(1)，则在最后一个单词前添加星号以进行前缀扩展（例如，`*word`）
- `append`: 布尔值（在 SQL 中为 0/1）。如果为 true(1)，则在最后一个单词后添加星号以进行后缀扩展（例如，`word*`）
- `expansion_len`: 在最后一个单词中扩展的字符数。默认：`10`

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
* [此演示](https://github.manticoresearch.com/manticoresoftware/manticoresearch) 演示了自动补全功能：
  ![自动补全示例](autocomplete_github_demo.png){.scale-0.7}
* 关于模糊搜索和自动补全的博客文章 - https://manticoresearch.com/blog/new-fuzzy-search-and-autocomplete/

## 替代的自动补全方法

虽然 `CALL AUTOCOMPLETE` 是大多数用例推荐的方法，但 Manticore 还支持其他可控和可自定义的方法来实现自动补全功能：

##### 自动补全句子
要自动补全句子，您可以使用 [infixed search](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)。您可以通过提供文档字段的开头找到其结尾，并且：
* 使用 [全文通配符运算符](../Searching/Full_text_matching/Operators.md) `*` 匹配任何字符
* 可选地使用 `^` 从字段的开头开始
* 可选地使用 `""` 进行短语匹配
* 使用 [结果高亮显示](../Searching/Highlighting.md)

There is an [article about it in our blog](https://manticoresearch.com/blog/simple-autocomplete-with-manticore/) and an [interactive course](https://play.manticoresearch.com/simpleautocomplete/). A quick example is:
* 假设你有一份文档： `My cat loves my dog. The cat (Felis catus) is a domestic species of small carnivorous mammal.`
* 然后你可以使用 `^`, `""`, 和 `*`，所以用户在输入时，你可以进行如下查询： `^"m*"`, `^"my *"`, `^"my c*"`, `^"my ca*"` 等等
* 这将找到文档，如果你还进行 [高亮显示](../Searching/Highlighting.md)，你将得到类似于： `<b>My cat</b> loves my dog. The cat ( ...`

##### 自动完成一个词
在某些情况下，你所需要的只是自动完成一个单词或几个单词。在这种情况下，你可以使用 `CALL KEYWORDS`。

### CALL KEYWORDS
`CALL KEYWORDS` 可以通过 SQL 接口获取，提供了一种检查关键词是如何被标记化的方式，或获取特定关键词的标记化形式。如果表启用了 [中缀](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)，它允许你快速找到给定关键词的可能结尾，使其适合自动完成功能。

这是一般中缀搜索的一个很好的替代方案，因为它提供了更高的性能，因为它只需要表的字典，而不需要文档本身。

### 一般语法
<!-- example keywords -->
```sql
CALL KEYWORDS(text, table [, options])
```
`CALL KEYWORDS` 语句将文本划分为关键词。它返回关键词的标记化和标准化形式，如果需要，可以返回关键词统计信息。此外，它提供了每个关键词在查询中的位置以及当表启用 [词形还原器](../Creating_a_table/NLP_and_tokenization/Morphology.md) 时所有形式的标记化关键词。

| 参数 | 描述 |
| - | - |
| text | 要分解为关键词的文本 |
| table | 用于获取文本处理设置的表名 |
| 0/1 作为 stats | 显示关键词统计，默认是 0 |
| 0/1 作为 fold_wildcards | 折叠通配符，默认是 0 |
| 0/1 作为 fold_lemmas | 折叠形态学词根，默认是 0 |
| 0/1 作为 fold_blended | 折叠混合词，默认是 0 |
| N 作为 expansion_limit | 覆盖服务器配置中定义的 [expansion_limit](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit)，默认是 0（使用配置文件中的值） |
| docs/hits 作为 sort_mode | 按 'docs' 或 'hits' 排序输出结果。默认情况下不应用排序。 |
| jieba_mode | 查询的 Jieba 分词模式。有关更多详细信息，请参见 [jieba_mode](Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode) |

示例显示了如果假设用户试图获取 "my cat ..." 的自动完成是如何工作的。因此在应用程序方面，你所需要做的就是为每个新单词从“标准化”列中建议用户的结尾。按 hits 或 docs 排序通常是有意义的，使用 `'hits' as sort_mode` 或 `'docs' as sort_mode`。

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
There is a nice trick how you can improve the above algorithm - use [bigram_index](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index). When you have it enabled for the table what you get in it is not just a single word, but each pair of words standing one after another indexed as a separate token.

This allows to predict not just the current word's ending, but the next word too which is especially beneficial for the purpose of autocomplete.

<!-- intro -->
##### 示例：

<!-- request Examples -->

```sql
MySQL [(none)]> CALL KEYWORDS('m*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | m*        | my         | 1    | 2    |
| 1    | m*        | 哺乳动物     | 1    | 1    |
| 1    | m*        | 我的猫     | 1    | 1    |
| 1    | m*        | 我的狗     | 1    | 1    |
+------+-----------+------------+------+------+

MySQL [(none)]> CALL KEYWORDS('my*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | my*       | my         | 1    | 2    |
| 1    | my*       | 我的猫     | 1    | 1    |
| 1    | my*       | 我的狗     | 1    | 1    |
+------+-----------+------------+------+------+

MySQL [(none)]> CALL KEYWORDS('c*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+--------------------+------+------+
| qpos | tokenized | normalized         | docs | hits |
+------+-----------+--------------------+------+------+
| 1    | c*        | 猫                | 1    | 2    |
| 1    | c*        | 食肉动物        | 1    | 1    |
| 1    | c*        | 食肉哺乳动物 | 1    | 1    |
| 1    | c*        | 猫 felus          | 1    | 1    |
| 1    | c*        | 猫 loves          | 1    | 1    |
| 1    | c*        | catus              | 1    | 1    |
| 1    | c*        | catus is           | 1    | 1    |
+------+-----------+--------------------+------+------+

MySQL [(none)]> CALL KEYWORDS('ca*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+--------------------+------+------+
| qpos | tokenized | normalized         | docs | hits |
+------+-----------+--------------------+------+------+
| 1    | ca*       | 猫                | 1    | 2    |
| 1    | ca*       | 食肉动物        | 1    | 1    |
| 1    | ca*       | 食肉哺乳动物 | 1    | 1    |
| 1    | ca*       | 猫 felus          | 1    | 1    |
| 1    | ca*       | 猫 loves          | 1    | 1    |
| 1    | ca*       | catus              | 1    | 1    |
| 1    | ca*       | catus is           | 1    | 1    |
+------+-----------+--------------------+------+------+

MySQL [(none)]> CALL KEYWORDS('cat*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | cat*      | 猫        | 1    | 2    |
| 1    | cat*      | 猫 felus  | 1    | 1    |
| 1    | cat*      | 猫 loves  | 1    | 1    |
| 1    | cat*      | catus      | 1    | 1    |
| 1    | cat*      | catus is   | 1    | 1    |
+------+-----------+------------+------+------+
```
<!-- end -->

`CALL KEYWORDS` 支持分布式表，因此无论您的数据集有多大，您都可以从中受益。
<!-- proofread -->
