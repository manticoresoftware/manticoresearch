# 搜索和排序函数

### BM25A()
`BM25A(k1,b)` 返回精确的 `BM25A()` 值。需要 `expr` 排序器并启用 `index_field_lengths`。参数 `k1` 和 `b` 必须是浮点数。

### BM25F()
`BM25F(k1, b, {field=weight, ...})` 返回精确的 `BM25F()` 值，且需要启用 `index_field_lengths`。同样需要 `expr` 排序器。参数 `k1` 和 `b` 必须是浮点数。

### EXIST()
用默认值替代不存在的列。它返回由 'attr-name' 指定的属性值，或者如果该属性不存在，则返回 'default-value'。不支持 STRING 或 MVA 属性。此函数在搜索多个具有不同模式的表时非常有用。

```sql
SELECT *, EXIST('gid', 6) as cnd FROM i1, i2 WHERE cnd>5
```

### MIN_TOP_SORTVAL()
如果排序键是浮点数，则返回当前 top-N 匹配中排名最差元素的排序键值，否则返回 0。

### MIN_TOP_WEIGHT()
返回当前 top-N 匹配中排名最差元素的权重。

### PACKEDFACTORS()
`PACKEDFACTORS()` 可用于查询中显示匹配期间计算的所有加权因子，或提供二进制属性以创建自定义排序 UDF。此函数仅在指定表达式排序器且查询不是全扫描时有效，否则返回错误。`PACKEDFACTORS()` 可以接受一个可选参数，禁用 ATC 排序因子计算：`PACKEDFACTORS({no_atc=1})`。计算 ATC 会显著降低查询处理速度，因此如果您需要查看排序因子但不需要 ATC，此选项非常有用。`PACKEDFACTORS()` 还可以输出 JSON 格式：`PACKEDFACTORS({json=1})`。以下分别展示了键值对和 JSON 格式的输出。（注意下面的示例为便于阅读进行了换行；实际返回值为单行。）

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

此函数可用于在 UDF 中实现自定义排序函数，如下所示：

```sql
SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
FROM my_index
WHERE match('hello')
ORDER BY r DESC
OPTION ranker=expr('1');
```

其中 `CUSTOM_RANK()` 是在 UDF 中实现的函数。它应声明一个 `SPH_UDF_FACTORS` 结构（定义于 sphinxudf.h），初始化该结构，在使用前解包因子到其中，使用后再反初始化，如下所示：

```sql
SPH_UDF_FACTORS factors;
sphinx_factors_init(&factors);
sphinx_factors_unpack((DWORD*)args->arg_values[0], &factors);
// ... can use the contents of factors variable here ...
sphinx_factors_deinit(&factors);
```

`PACKEDFACTORS()` 数据在所有查询阶段均可用，而不仅限于初始匹配和排序阶段。这使得 `PACKEDFACTORS()` 另一个特别有趣的应用成为可能：重新排序。

在上面的示例中，我们使用了基于表达式的排序器和一个虚拟表达式，并按我们 UDF 计算的值对结果集进行排序。换句话说，我们用 UDF 对所有结果进行排序。现在，假设我们的 UDF 计算非常昂贵，吞吐量仅为每秒 10,000 次调用。如果查询匹配了 1,000,000 个文档，我们希望使用更简单的表达式进行大部分排序，以保持合理的性能。然后，我们只对少数顶级结果（比如前 100 个）应用昂贵的 UDF。换句话说，我们用更简单的排序函数构建前 100 个结果，然后用更复杂的函数重新排序它们。这可以通过子查询实现：

```sql
SELECT * FROM (
    SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
    FROM my_index WHERE match('hello')
    OPTION ranker=expr('sum(lcs)*1000+bm25')
    ORDER BY WEIGHT() DESC
    LIMIT 100
) ORDER BY r DESC LIMIT 10
```

在此示例中，基于表达式的排序器为每个匹配文档调用以计算 `WEIGHT()`，因此调用了 1,000,000 次。然而，UDF 计算可以推迟到外层排序，只对内层限制的前 100 个 `WEIGHT()` 匹配执行。这意味着 UDF 只调用 100 次。最后，按 UDF 值选出前 10 个匹配并返回给应用程序。

作为参考，在分布式设置中，`PACKEDFACTORS()` 数据以二进制格式从代理发送到主节点。这使得在主节点上实现额外的重新排序过程在技术上成为可能（如果需要）。

在 SQL 中使用但未从任何 UDF 调用时，`PACKEDFACTORS()` 的结果格式为纯文本，可用于手动评估排序因子。注意此功能当前不被 Manticore API 支持。


### REMOVE_REPEATS()
`REMOVE_REPEATS ( result_set, column, offset, limit )` - 移除具有相同 'column' 值的重复调整行。

```sql
SELECT REMOVE_REPEATS((SELECT * FROM dist1), gid, 0, 10)
```
注意，`REMOVE_REPEATS` 不影响 [搜索查询元信息](../Node_info_and_management/SHOW_META.md#SHOW-META)中的 `total_found`。

### WEIGHT()
`WEIGHT()` 函数返回计算出的匹配分数。如果未指定排序，则结果按 `WEIGHT()` 提供的分数降序排序。在此示例中，我们先按权重排序，再按整数属性排序。

上述搜索执行简单匹配，所有词都必须出现。但我们可以做更多（这只是一个简单示例）：

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

这里，我们搜索四个词，但即使只找到其中三个词也可匹配。包含所有词的文档排名更高。

### ZONESPANLIST()
`ZONESPANLIST()` 函数返回匹配的区域跨度对。每对包含匹配区域跨度标识符、冒号和匹配区域跨度的顺序号。例如，如果文档内容为 `<emphasis role="bold"><i>text</i> the <i>text</i></emphasis>`，且查询为 `'ZONESPAN:(i,b) text'`，则 `ZONESPANLIST()` 返回字符串 `"1:1 1:2 2:1"`，表示第一个区域跨度在跨度 1 和 2 中匹配了 "text"，第二个区域跨度仅在跨度 1 中匹配。

### QUERY()
`QUERY()` 返回当前搜索查询。`QUERY()` 是一个后限制表达式，旨在与 [SNIPPET()](../Functions/String_functions.md#SNIPPET%28%29) 一起使用。

表函数是一种用于查询后结果集处理的机制。表函数以任意结果集作为输入，并返回一个新的、处理过的结果集作为输出。第一个参数应为输入结果集，但表函数可以选择性地接受和处理更多参数。表函数可以完全改变结果集，包括其模式。目前，仅支持内置表函数。表函数适用于外部 `SELECT` 和[嵌套 SELECT](../Searching/Sub-selects.md)。

<!-- proofread -->

