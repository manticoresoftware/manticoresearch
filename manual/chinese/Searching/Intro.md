# 使用 Manticore Search 进行搜索简介

搜索是 Manticore Search 的核心功能。您可以：
* 执行[全文搜索](../Searching/Full_text_matching/Basic_usage.md#MATCH) 并实现搜索结果的[高亮显示](../Searching/Highlighting.md)
* 执行[k 最近邻搜索](../Searching/KNN.md)
* 应用[非全文过滤](../Searching/Filters.md)
* 使用[表达式](../Searching/Expressions.md)进行过滤
* 利用各种[搜索选项](../Searching/Options.md)
* 使用[多查询](../Searching/Multi-queries.md)和[子选择](../Searching/Sub-selects.md)
* 对搜索结果进行[聚合](../Searching/Grouping.md)和[分面搜索](../Searching/Faceted_search.md)
* 以及更多功能

## 通用语法

**SQL**：
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
    [FACET {expr_list} [BY {expr_list}] [DISTINCT {field_name}] [ORDER BY {expr | FACET()} {ASC | DESC}] [LIMIT [offset,] count]]

```

**JSON**：
```json
POST /search
{
    "table" : "table_name",
    "options":
    {
      ...
    }
}
```
<!-- proofread -->

