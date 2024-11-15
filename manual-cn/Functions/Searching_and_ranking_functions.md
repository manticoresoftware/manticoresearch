# 搜索和排序函数

### BM25A()
`BM25A(k1,b)` 返回精确的 `BM25A()` 值。需要 `expr` 排序器，并启用 `index_field_lengths`。参数 `k1` 和 `b` 必须是浮点数。

### BM25F()
`BM25F(k1, b, {field=weight, ...})` 返回精确的 `BM25F()` 值，并要求启用 `index_field_lengths`。也需要 `expr`排序器。参数 `k1` 和 `b` 必须是浮点数。

### EXIST()
用于替换不存在的列，返回由 'attr-name' 指定的属性值，如果该属性不存在则返回 'default-value'。不支持字符串或 MVA 属性。此函数在处理多个表且表结构不同的情况下非常有用。

```sql
SELECT *, EXIST('gid', 6) as cnd FROM i1, i2 WHERE cnd>5
```

### MIN_TOP_SORTVAL()
返回当前 top-N 匹配中排名最差的元素的排序键值（如果排序键是浮点数），否则返回 0。

### MIN_TOP_WEIGHT()
返回当前 top-N 匹配中排名最差的元素的权重。

### PACKEDFACTORS()
`PACKEDFACTORS()` 可用于在匹配期间显示所有计算的权重因子，或提供二进制属性以创建自定义排序的 UDF。此函数仅在指定了表达式排序器且查询不是全扫描时可用；否则会返回错误。`PACKEDFACTORS()` 可以接受一个可选参数，用于禁用 ATC 排序因子计算：`PACKEDFACTORS({no_atc=1})`。计算 ATC 会显著降低查询处理速度，因此如果只需要查看排序因子而不需要 ATC，这个选项很有用。`PACKEDFACTORS()` 也可以输出 JSON 格式：`PACKEDFACTORS({json=1})`。

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

可以通过子查询进行重新排序，以减少计算次数：

```sql
SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
FROM my_index
WHERE match('hello')
ORDER BY r DESC
OPTION ranker=expr('1');
```

当 `CUSTOM_RANK()` 是在 UDF 中实现的函数时，它应声明一个 `SPH_UDF_FACTORS` 结构体（定义在 sphinxudf.h 中），在使用之前初始化该结构体，解包其中的因素，并在使用后进行解初始化，代码如下：

```sql
SPH_UDF_FACTORS factors;
sphinx_factors_init(&factors);
sphinx_factors_unpack((DWORD*)args->arg_values[0], &factors);
// ... can use the contents of factors variable here ...
sphinx_factors_deinit(&factors);
```

`PACKEDFACTORS()` 数据在所有查询阶段都可用，不仅限于初始匹配和排名阶段。这使得 `PACKEDFACTORS()` 可以应用于重新排名。

在上述示例中，我们使用基于表达式的排序器，并通过我们的 UDF 计算的值对结果集进行排序。换句话说，我们用 UDF 对所有结果进行了排名。假设我们的 UDF 计算非常昂贵，每秒只能处理 10,000 次调用。如果查询匹配了 1,000,000 个文档，我们可能希望使用更简单的表达式进行大部分排名，以保持合理的性能。然后我们可以对少量顶级结果（例如前 100 个）应用昂贵的 UDF。也就是说，我们使用较简单的排序函数构建前 100 个结果，然后用更复杂的函数对这些结果重新排名。可以通过子查询完成这一操作：

```sql
SELECT * FROM (
    SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
    FROM my_index WHERE match('hello')
    OPTION ranker=expr('sum(lcs)*1000+bm25')
    ORDER BY WEIGHT() DESC
    LIMIT 100
) ORDER BY r DESC LIMIT 10
```

在此示例中，表达式排序器为每个匹配的文档调用以计算 `WEIGHT()`，因此它被调用了 1,000,000 次。然而，UDF 计算可以推迟到外部排序，并且只会对根据内部限制获得的前 100 个结果调用 UDF。这样，UDF 只需调用 100 次。最后，应用程序会根据 UDF 值选出前 10 个匹配项并返回。

在分布式环境中，`PACKEDFACTORS()` 数据以二进制格式从代理发送到主节点。这使得在主节点上实现额外的重新排名成为可能。

当在 SQL 中使用而不通过任何 UDF 调用时，`PACKEDFACTORS()` 的结果会以纯文本格式输出，可以手动评估排序因素。请注意，该功能目前不支持 Manticore API。


### REMOVE_REPEATS()
`REMOVE_REPEATS ( result_set, column, offset, limit )` 删除具有相同 'column' 值的重复行。

```sql
SELECT REMOVE_REPEATS((SELECT * FROM dist1), gid, 0, 10)
```
请注意，`REMOVE_REPEATS` 不会影响 [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_found`。

### WEIGHT()
`WEIGHT()` 函数返回计算出的匹配评分。如果没有指定排序，结果将按 `WEIGHT()` 提供的分数以降序排序。在以下示例中，我们首先按权重排序，然后按整数属性排序。

上面的搜索执行了一个简单的匹配，要求所有单词都必须出现。然而，我们可以做得更多（这是一个简单的示例）：

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

在此示例中，我们搜索了四个单词，但即使仅找到其中的三个单词，匹配也会发生。包含所有单词的文档将排名更高。

### ZONESPANLIST()
`ZONESPANLIST()` 函数返回匹配的区域跨度对。每对包含匹配的区域跨度标识符、冒号和匹配的区域跨度的顺序号。例如，如果文档内容为 `<emphasis role="bold"><i>text</i> the <i>text</i></emphasis>`，并且您查询 `'ZONESPAN:(i,b) text'`，则 `ZONESPANLIST()` 会返回字符串 `"1:1 1:2 2:1"`，这意味着第一个区域跨度在第 1 和第 2 个区域中匹配 "text"，第二个区域跨度仅在第 1 个区域中匹配。

### QUERY()
`QUERY()` 返回当前的搜索查询。`QUERY()` 是一个 postlimit 表达式，旨在与 [SNIPPET()](../Functions/String_functions.md#SNIPPET()) 一起使用。

表函数是一种用于查询结果集后处理的机制。表函数接受任意结果集作为输入并返回新的、经过处理的结果集。第一个参数应为输入结果集，但表函数还可以可选地处理更多参数。表函数可以完全更改结果集，包括其模式。目前仅支持内置的表函数。表函数适用于外部 `SELECT` 和 [嵌套 SELECT](../Searching/Sub-selects.md)。

<!-- proofread -->
