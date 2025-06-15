# 搜索和排序函数

### BM25A()
`BM25A(k1,b)` 返回精确的 `BM25A()` 值。需要使用 `expr` 排名器并启用 `index_field_lengths`。参数 `k1` 和 `b` 必须是浮点数。

### BM25F()
`BM25F(k1, b, {field=weight, ...})` 返回精确的 `BM25F()` 值，并且需要启用 `index_field_lengths`。同样需要使用 `expr` 排名器。参数 `k1` 和 `b` 必须是浮点数。

### EXIST()
用默认值替代不存在的列。返回指定属性 `'attr-name'` 的值，如果该属性不存在，则返回 `'default-value'`。不支持 STRING 或 MVA 属性。此函数在跨多个不同模式的表搜索时非常有用。

```sql
SELECT *, EXIST('gid', 6) as cnd FROM i1, i2 WHERE cnd>5
```

### MIN_TOP_SORTVAL()
如果排序键是浮点数，则返回当前 top-N 匹配中排名最差元素的排序键值，否则返回 0。

### MIN_TOP_WEIGHT()
返回当前 top-N 匹配中排名最差元素的权重。

### PACKEDFACTORS()
`PACKEDFACTORS()` 可用于查询中展示匹配时计算的所有加权因子，或提供二进制属性以创建自定义排名 UDF。此函数仅在指定表达式排名器且查询不是全表扫描时有效，否则返回错误。`PACKEDFACTORS()` 可以接受一个可选参数，用于禁用 ATC 排名因子计算：`PACKEDFACTORS({no_atc=1})`。计算 ATC 会显著降低查询性能，因此如果只需查看排名因子但不需要 ATC，此选项很有用。`PACKEDFACTORS()` 还可以输出 JSON 格式：`PACKEDFACTORS({json=1})`。下面示例展示了键值对及 JSON 格式的相应输出。（注意，下面示例为了可读性进行了换行；实际返回值为单行。）

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

此函数可用于在 UDF 中实现自定义排名函数，例如：

```sql
SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
FROM my_index
WHERE match('hello')
ORDER BY r DESC
OPTION ranker=expr('1');
```

其中 `CUSTOM_RANK()` 是 UDF 中实现的函数。该函数应声明一个 `SPH_UDF_FACTORS` 结构体（定义在 sphinxudf.h 中），初始化该结构体，在使用前将因子解包入其中，使用后再进行反初始化，如下所示：

```sql
SPH_UDF_FACTORS factors;
sphinx_factors_init(&factors);
sphinx_factors_unpack((DWORD*)args->arg_values[0], &factors);
// ... can use the contents of factors variable here ...
sphinx_factors_deinit(&factors);
```

`PACKEDFACTORS()` 数据在所有查询阶段均可用，不仅仅是在初始匹配和排名阶段。这使得 `PACKEDFACTORS()` 另一个特别有趣的应用成为可能：重新排序。

上述例子中，我们使用了一个基于表达式的排名器和一个虚拟表达式，并按 UDF 计算的值排序结果集。换言之，我们使用 UDF 来对所有结果排名。现在假设例子中我们的 UDF 计算开销极大，吞吐只有每秒 10,000 次调用。如果查询匹配了 1,000,000 个文档，我们希望用更简单的表达式做大部分排序以保持合理性能，然后只将昂贵的 UDF 应用于最前面的少量结果，比如前 100 个。换句话说，我们使用简单的排名函数构建前 100 名结果后，再用更复杂的函数重新排序这些结果。这可以通过子查询实现：

```sql
SELECT * FROM (
    SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
    FROM my_index WHERE match('hello')
    OPTION ranker=expr('sum(lcs)*1000+bm25')
    ORDER BY WEIGHT() DESC
    LIMIT 100
) ORDER BY r DESC LIMIT 10
```

在这个例子中，表达式排名器为每个匹配文档计算 `WEIGHT()`，总共调用了 1,000,000 次。然而，UDF 的计算被推迟到外层排序，仅对根据内层限制的 `WEIGHT()` 取得的前 100 个匹配执行。所以 UDF 只调用 100 次。最终，选出 UDF 值排名前 10 的结果并返回给应用。

作为参考，在分布式环境下，`PACKEDFACTORS()` 数据以二进制格式从代理节点发送到主节点。这使得在主节点上实现额外重新排序过程成为技术上可行。

在 SQL 中使用但不在任何 UDF 中调用时，`PACKEDFACTORS()` 的结果将格式化为纯文本，可以用于手动评估排名因子。注意，Manticore API 当前不支持该功能。


### REMOVE_REPEATS()
`REMOVE_REPEATS ( result_set, column, offset, limit )` - 移除 'column' 列值相同的重复调整行。

```sql
SELECT REMOVE_REPEATS((SELECT * FROM dist1), gid, 0, 10)
```
注意，`REMOVE_REPEATS` 不会影响 [搜索查询元信息](../Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_found`。

### WEIGHT()
`WEIGHT()` 函数返回计算后的匹配得分。如果未指定排序方式，结果默认按 `WEIGHT()` 提供的得分降序排序。以下示例先按权重排序，再按整数属性排序。

上述搜索执行了简单匹配，所有词都必须存在。不过，我们可以做得更多（这只是一个简单示例）：

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

这里我们搜索四个词，但即使只有四个词中有三个匹配，也会被视为匹配。包含所有词的文档得分更高。

### ZONESPANLIST()
`ZONESPANLIST()` 函数返回匹配的区域跨度对。每对包含匹配区域跨度标识符、冒号、及匹配区域跨度的序号。例如，若文档内容为 `<emphasis role="bold"><i>text</i> the <i>text</i></emphasis>`，查询 `'ZONESPAN:(i,b) text'`，则 `ZONESPANLIST()` 返回字符串 `"1:1 1:2 2:1"`，其中表示第一个区域跨度在跨度 1 和 2 匹配了 "text"，第二个区域跨度只在跨度 1 匹配了。

### QUERY()
`QUERY()` 返回当前的搜索查询。`QUERY()` 是后限制表达式，通常与 [SNIPPET()](../Functions/String_functions.md#SNIPPET%28%29) 一起使用。

表函数是一种用于查询结果集后处理的机制。表函数以任意结果集作为输入，并返回一个新的、经过处理的结果集。第一个参数应该是输入结果集，但表函数可以可选地接受和处理更多参数。表函数可以完全改变结果集，包括其架构。目前仅支持内置表函数。表函数适用于外部 `SELECT` 和 [嵌套 SELECT](../Searching/Sub-selects.md)。

<!-- proofread -->

