# 搜索和排名函数

### BM25A()
`BM25A(k1,b)` 返回精确的 `BM25A()` 值。需要 `expr` 排序器并且启用了 `index_field_lengths`。参数 `k1` 和 `b` 必须是浮点数。

### BM25F()
`BM25F(k1, b, {field=weight, ...})` 返回精确的 `BM25F()` 值，并且需要启用 `index_field_lengths`。还需要 `expr` 排序器。参数 `k1` 和 `b` 必须是浮点数。

### EXIST()
用默认值替换不存在的列。它返回由 'attr-name' 指定的属性值，或者如果该属性不存在，则返回 'default-value'。STRING 或 MVA 属性不支持。此函数在搜索具有不同模式的多个表时很有用。

```sql
SELECT *, EXIST('gid', 6) as cnd FROM i1, i2 WHERE cnd>5
```

### MIN_TOP_SORTVAL()
如果排序键是浮点数，则返回当前前 N 匹配中最差排名元素的排序键值；否则返回 0。

### MIN_TOP_WEIGHT()
返回当前前 N 匹配中最差排名元素的权重。

### PACKEDFACTORS()
`PACKEDFACTORS()` 可以用于查询中显示匹配或匹配期间计算的所有加权因子，也可以提供二进制属性以创建自定义排名 UDF。此函数仅在指定表达式排序器且查询不是全扫描时有效；否则返回错误。`PACKEDFACTORS()` 可以接受一个可选参数来禁用 ATC 排名因子计算：`PACKEDFACTORS({no_atc=1})`。计算 ATC 显著减慢了查询处理速度，因此如果只需要查看排名因子但不需要 ATC，此选项很有用。`PACKEDFACTORS()` 还可以输出为 JSON 格式：`PACKEDFACTORS({json=1})`。以下是在键值对或 JSON 格式下的相应输出示例。（注意，下面的示例是为了可读性而换行的；实际返回值会是一行。）

```sql
mysql> SELECT id, PACKEDFACTORS() FROM test1
    -> WHERE MATCH('test one') OPTION ranker=expr('1') \G
*************************** 1\. row ***************************
             id: 1
packedfactors(): bm25=569, bm25a=0.617197, field_mask=2, doc_word_count=2,
    field1=(lcs=1, hit_count=2, word_count=2, tf_idf=0.152356,
        min_idf=-0.062982, max_idf=0.215338, sum_idf=0.152356, min_hit_pos=4,
        min_best_span_pos=4, exact_hit=0, max_window_hits=1, min_gaps=2,
        exact_order=1, lccs=1, wlccs=0.215338, atc=-0.003974),
    word0=(tf=1, idf=-0.062982),
    word1=(tf=1, idf=0.215338)
1 row in set (0.00 sec)
```

```sql
mysql> SELECT id, PACKEDFACTORS({json=1}) FROM test1
    -> WHERE MATCH('test one') OPTION ranker=expr('1') \G
*************************** 1\. row ***************************
                     id: 1
packedfactors({json=1}):
{

    "bm25": 569,
    "bm25a": 0.617197,
    "field_mask": 2,
    "doc_word_count": 2,
    "fields": [
        {
            "lcs": 1,
            "hit_count": 2,
            "word_count": 2,
            "tf_idf": 0.152356,
            "min_idf": -0.062982,
            "max_idf": 0.215338,
            "sum_idf": 0.152356,
            "min_hit_pos": 4,
            "min_best_span_pos": 4,
            "exact_hit": 0,
            "max_window_hits": 1,
            "min_gaps": 2,
            "exact_order": 1,
            "lccs": 1,
            "wlccs": 0.215338,
            "atc": -0.003974
        }
    ],
    "words": [
        {
            "tf": 1,
            "idf": -0.062982
        },
        {
            "tf": 1,
            "idf": 0.215338
        }
    ]

}
1 row in set (0.01 sec)
```

此函数可用于在 UDF 中实现自定义排名函数，如下所示：

```sql
SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
FROM my_index
WHERE match('hello')
ORDER BY r DESC
OPTION ranker=expr('1');
```

其中 `CUSTOM_RANK()` 是在 UDF 中实现的一个函数。它应该声明一个 `SPH_UDF_FACTORS` 结构（定义在 sphinxudf.h 中），初始化此结构，在使用前将因子解包到其中，并在使用后进行反初始化，如下所示：

```sql
SPH_UDF_FACTORS factors;
sphinx_factors_init(&factors);
sphinx_factors_unpack((DWORD*)args->arg_values[0], &factors);
// ... can use the contents of factors variable here ...
sphinx_factors_deinit(&factors);
```

`PACKEDFACTORS()` 数据在所有查询阶段都可用，而不仅仅是初始匹配和排名阶段。这使 `PACKEDFACTORS()` 的另一个特别有趣的用途成为可能：重新排名。

在上面的例子中，我们使用了一个基于表达式的排序器和一个假表达式，并按我们的 UDF 计算出的值对结果集进行了排序。换句话说，我们使用 UDF 对所有结果进行了排序。现在，假设为了举例说明，我们的 UDF 计算非常昂贵，吞吐量仅为每秒 10,000 次调用。如果我们查询匹配了 1,000,000 个文档，我们希望使用一个更简单的表达式来进行大部分排序，以便保持合理的性能。然后，我们将只对前 100 个结果应用昂贵的 UDF。换句话说，我们将使用一个更简单的排名函数构建前 100 个结果，然后再用一个更复杂的函数对其进行重新排序。这可以通过子查询完成：

```sql
SELECT * FROM (
    SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
    FROM my_index WHERE match('hello')
    OPTION ranker=expr('sum(lcs)*1000+bm25')
    ORDER BY WEIGHT() DESC
    LIMIT 100
) ORDER BY r DESC LIMIT 10
```

在这个例子中，对于每个匹配的文档，基于表达式的排序器都会计算 `WEIGHT()`，因此会被调用 1,000,000 次。然而，UDF 计算可以推迟到外部排序，并且只会在 `WEIGHT()` 的前 100 个匹配中执行，根据内部限制。这意味着 UDF 将只会被调用 100 次。最后，根据 UDF 值选择并返回前 10 个结果给应用程序。

作为参考，在分布式设置中，`PACKEDFACTORS()` 数据将以二进制格式从代理发送到主节点。这使得在主节点上实现额外的重新排序步骤成为技术上可行的。

当在 SQL 中使用但未从任何 UDF 调用时，`PACKEDFACTORS()` 的结果以纯文本格式呈现，可用于手动评估排名因子。请注意，此功能目前不被 Manticore API 支持。


### REMOVE_REPEATS()
`REMOVE_REPEATS ( result_set, column, offset, limit )` - 删除具有相同 'column' 值的重复调整行。

```sql
SELECT REMOVE_REPEATS((SELECT * FROM dist1), gid, 0, 10)
```
请注意，`REMOVE_REPEATS` 不会影响 [搜索查询元信息](../Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_found`。

### WEIGHT()
`WEIGHT()` 函数返回计算出的匹配得分。如果没有指定排序，则结果按 `WEIGHT()` 提供的得分降序排序。在上面的示例中，我们首先按权重排序，然后按整数属性排序。

上述搜索执行简单的匹配，所有单词都需要存在。但是我们可以做更多（这只是简单示例）：

```sql
mysql> SELECT *,WEIGHT() FROM testrt WHERE MATCH('"list of business laptops"/3');
+------+------+-------------------------------------+---------------------------+----------+
| id   | gid  | title                               | content                   | weight() |
+------+------+-------------------------------------+---------------------------+----------+
|    1 |   10 | List of HP business laptops         | Elitebook Probook         |     2397 |
|    2 |   10 | List of Dell business laptops       | Latitude Precision Vostro |     2397 |
|    3 |   20 | List of Dell gaming laptops         | Inspirion Alienware       |     2375 |
|    5 |   30 | List of ASUS ultrabooks and laptops | Zenbook Vivobook          |     2375 |
+------+------+-------------------------------------+---------------------------+----------+
4 rows in set (0.00 sec)


mysql> SHOW META;
+----------------+----------+
| Variable_name  | Value    |
+----------------+----------+
| total          | 4        |
| total_found    | 4        |
| total_relation | eq       |
| time           | 0.000    |
| keyword[0]     | list     |
| docs[0]        | 5        |
| hits[0]        | 5        |
| keyword[1]     | of       |
| docs[1]        | 4        |
| hits[1]        | 4        |
| keyword[2]     | business |
| docs[2]        | 2        |
| hits[2]        | 2        |
| keyword[3]     | laptops  |
| docs[3]        | 5        |
| hits[3]        | 5        |
+----------------+----------+
16 rows in set (0.00 sec)
```

在这里，我们搜索四个单词，但如果只有四个单词中的三个被找到，也可以匹配。搜索将对包含所有单词的文档进行更高排名。

### ZONESPANLIST()
`ZONESPANLIST()` 函数返回匹配区间的对。每个对包含匹配区间的标识符、冒号以及匹配区间的顺序编号。例如，如果文档内容为 `<emphasis role="bold"><i>text</i> the <i>text</i></emphasis>`，并且查询为 `'ZONESPAN:(i,b) text'`，则 `ZONESPANLIST()` 将返回字符串 `"1:1 1:2 2:1"`，表示第一个区间匹配了在区间 1 和 2 中的 "text"，第二个区间仅在区间 1 中匹配了 "text"。

### QUERY()
`QUERY()` 返回当前搜索查询。`QUERY()` 是一个 postlimit 表达式，并且旨在与 [SNIPPET()](../Functions/String_functions.md#SNIPPET%28%29) 一起使用。

表函数是一种用于查询后结果集处理的机制。表函数接收任意结果集作为输入，并返回一个新的、经过处理的结果集作为输出。第一个参数应为输入结果集，但表函数可以选择性地接受和处理更多参数。表函数可以完全改变结果集，包括其模式。目前，仅支持内置表函数。表函数适用于外部`SELECT`和[嵌套SELECT](../Searching/Sub-selects.md)。

<!-- proofread -->

