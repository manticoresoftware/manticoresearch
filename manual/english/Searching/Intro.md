# Introduction into searching with Manticore Search

Searching is a core feature of Manticore Search. You can:
* Perform [full-text search](../Searching/Full_text_matching/Basic_usage.md#MATCH) and implement search result [highlighting](../Searching/Highlighting.md)
* Perform [k-nearest neighbor search](../Searching/KNN.md)
* Apply [non-full-text filtering](../Searching/Filters.md)
* Use [expressions](../Searching/Expressions.md) for filtering
* Utilize various [search options](../Searching/Options.md)
* Employ [multi-queries](../Searching/Multi-queries.md) and [sub-selects](../Searching/Sub-selects.md)
* Conduct [aggregations](../Searching/Grouping.md) and [faceting](../Searching/Faceted_search.md) of search results
* And much more

## General syntax

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
    [FACET {expr_list} [BY {expr_list}] [DISTINCT {field_name}] [ORDER BY {expr | FACET()} {ASC | DESC}] [LIMIT [offset,] count]]

```

**JSON**:
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
