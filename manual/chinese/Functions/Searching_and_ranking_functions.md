# 搜索和排名函数

### BM25A()
`BM25A(k1,b)` 返回精确的 `BM25A()` 值。需要 `expr` 排名器并启用 `index_field_lengths`。参数 `k1` 和 `b` 必须是浮点数。

### BM25F()
`BM25F(k1, b, {field=weight, ...})` 返回精确的 `BM25F()` 值，并要求启用 `index_field_lengths`。同时需要 `expr` 排名器。参数 `k1` 和 `b` 必须是浮点数。

### EXIST()
用默认值替代不存在的列。它返回由 'attr-name' 指定的属性值，或者如果该属性不存在则返回 'default-value'。不支持字符串或多值属性。此函数在搜索多个具有不同架构的表时非常有用。

```sql
SELECT *, EXIST('gid', 6) as cnd FROM i1, i2 WHERE cnd>5
```

### MIN_TOP_SORTVAL()
如果排序键是浮点值，返回当前前N匹配中排名最差元素的排序键值，其他情况下返回0。

### MIN_TOP_WEIGHT()
返回当前前N匹配中排名最差元素的权重。

### PACKEDFACTORS()
`PACKEDFACTORS()` 可用于查询中显示匹配时计算的所有权重因子，或提供用于创建自定义排名 UDF 的二进制属性。此函数仅在指定表达式排名器且查询不是全扫描时有效；否则，将返回错误。 `PACKEDFACTORS()` 可以接受一个可选参数，禁用 ATC 排名因子计算：`PACKEDFACTORS({no_atc=1})`。计算 ATC 会显著减慢查询处理，因此如果您需要查看排名因子却不需要 ATC，此选项会非常有用。 `PACKEDFACTORS()` 还可以以 JSON 格式输出：`PACKEDFACTORS({json=1})`。以下显示了以键值对或 JSON 格式的相应输出。（请注意，下面的示例因可读性而换行；实际返回值将是单行。）

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

其中 `CUSTOM_RANK()` 是在 UDF 中实现的函数。它应声明一个 `SPH_UDF_FACTORS` 结构（在 sphinxudf.h 中定义），在使用之前初始化该结构，并在之后进行反初始化，如下所示：

```sql
SPH_UDF_FACTORS factors;
sphinx_factors_init(&factors);
sphinx_factors_unpack((DWORD*)args->arg_values[0], &factors);
// ... 可以在这里使用 factors 变量的内容 ...
sphinx_factors_deinit(&factors);
```

`PACKEDFACTORS()` 数据在所有查询阶段都可用，而不仅仅是在初始匹配和排名通过期间。这使 `PACKEDFACTORS()` 的另一个特别有趣的应用成为可能：重新排名。

在上面的示例中，我们使用基于表达式的排名器和一个虚拟表达式，并按我们的 UDF 计算的值对结果集进行排序。换句话说，我们使用 UDF 对所有结果进行排名。现在，假设为了示例，我们的 UDF 计算非常昂贵，吞吐量只有每秒 10,000 次调用。如果我们的查询匹配 1,000,000 个文档，我们希望使用更简单的表达式来完成大部分排名，以保持合理的性能。然后，我们只需对少数前结果应用昂贵的 UDF，比如说前 100 个结果。换句话说，我们将使用更简单的排名函数构建前 100 个结果，然后用更复杂的函数对其进行重新排名。这可以通过子选择来完成：

```sql
SELECT * FROM (
    SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
    FROM my_index WHERE match('hello')
    OPTION ranker=expr('sum(lcs)*1000+bm25')
    ORDER BY WEIGHT() DESC
    LIMIT 100
) ORDER BY r DESC LIMIT 10
```
在这个例子中，基于表达式的排名器被调用以计算 `WEIGHT()`，因此它被调用了 1,000,000 次。然而，UDF 计算可以推迟到外部排序，并且仅在内限的情况下对前 100 个匹配项执行 `WEIGHT()`。这意味着 UDF 仅会被调用 100 次。最后，根据 UDF 值选择前 10 个匹配项并返回给应用程序。

作为参考，在分布式设置中，`PACKEDFACTORS()` 数据以二进制格式从代理发送到主节点。这使得在需要时在主节点上实现额外的重新排名遍历在技术上是可行的。

当在 SQL 中使用但不从任何 UDF 调用时，`PACKEDFACTORS()` 的结果格式化为纯文本，可以用来手动评估排名因素。请注意，此功能目前不被 Manticore API 支持。


### REMOVE_REPEATS()
`REMOVE_REPEATS ( result_set, column, offset, limit )` - 移除具有相同 'column' 值的重复调整行。

```sql
SELECT REMOVE_REPEATS((SELECT * FROM dist1), gid, 0, 10)
```
请注意，`REMOVE_REPEATS` 不会影响 [搜索查询元信息](../../Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_found`。

### WEIGHT()
`WEIGHT()` 函数返回计算得出的匹配分数。如果未指定排序，则结果按 `WEIGHT()` 提供的分数降序排序。在这个例子中，我们首先按权重排序，然后按一个整数属性排序。

上面的搜索执行简单匹配，所有单词都需要存在。然而，我们可以做更多（这只是一个简单的例子）：

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

在这里，我们搜索四个单词，但即使只找到四个单词中的三个，匹配也可以发生。该搜索将更高地排名包含所有单词的文档。

### ZONESPANLIST()
`ZONESPANLIST()` 函数返回匹配区域跨度的对。每对包含匹配区域跨度标识符、一个冒号和匹配区域跨度的序号。例如，如果文档读取 `<emphasis role="bold"><i>text</i> the <i>text</i></emphasis>`，并且您查询 `'ZONESPAN:(i,b) text'`，那么 `ZONESPANLIST()` 将返回字符串 `"1:1 1:2 2:1"`，这意味着第一个区域跨度匹配 "text" 在跨度 1 和 2，以及第二个区域跨度仅在跨度 1。

### QUERY()
`QUERY()` 返回当前搜索查询。`QUERY()` 是一个后限制表达式，旨在与 [SNIPPET()](../Functions/String_functions.md#SNIPPET%28%29) 一起使用。

表函数是一种用于查询结果集合处理的机制。表函数将任意结果集作为输入并返回一个新的处理集作为输出。第一个参数应该是输入结果集，但表函数可以选择接受并处理更多参数。表函数可以完全改变结果集，包括模式。目前，仅支持内置表函数。表函数适用于外部 `SELECT` 和 [嵌套 SELECT](../Searching/Sub-selects.md)。

<!-- proofread -->

