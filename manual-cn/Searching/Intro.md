# Manticore Search 搜索入门

搜索是 Manticore Search 的核心功能。你可以：



 执行[全文搜索](../Searching/Full_text_matching/Basic_usage.md#MATCH)并实现搜索结果的[高亮显示](../Searching/Highlighting.md)

* 执行[k-最近邻搜索](../Searching/KNN.md)

* 应用[非全文过滤](../Searching/Filters.md)

* 使用[表达式](../Searching/Expressions.md)进行过滤

* 利用各种[搜索选项](../Searching/Options.md)

* 使用[多查询](../Searching/Multi-queries.md)和[子查询](../Searching/Sub-selects.md)

* 进行搜索结果的[聚合](../Searching/Grouping.md)和[分面搜索](../Searching/Faceted_search.md)

* 以及更多功能



## 通用语法

**SQL**:
```sql
SELECT
    select_expr [, select_expr] ...
    [FROM tbl_name
        [{INNER | LEFT} JOIN tbl2_name]
        [WHERE where_condition]
        [GROUP BY {col_name | expr}, ... ]
        [HAVING where_condition]
        [ORDER BY {col_name | expr}
            [ASC | DESC], ... ]
    ]
    [LIMIT {[offset,] row_count | row_count OFFSET offset}]
    [OPTION option1[, option2] ...]

```

**JSON**:
```json
POST /search
{   
    "index" : "index_name",
    "options":   
    {
      ...
    }
}
```
<!-- proofread -->
