# Введение в поиск с Manticore Search

Поиск является основной функцией Manticore Search. Вы можете:
* Выполнять [полнотекстовый поиск](../Searching/Full_text_matching/Basic_usage.md#MATCH) и реализовывать [подсветку](../Searching/Highlighting.md) результатов поиска
* Выполнять [поиск k-ближайших соседей](../Searching/KNN.md)
* Применять [фильтрацию, не связанную с полнотекстовым поиском](../Searching/Filters.md)
* Использовать [выражения](../Searching/Expressions.md) для фильтрации
* Использовать различные [опции поиска](../Searching/Options.md)
* Использовать [мультизапросы](../Searching/Multi-queries.md) и [подзапросы](../Searching/Sub-selects.md)
* Выполнять [агрегации](../Searching/Grouping.md) и [фасетный поиск](../Searching/Faceted_search.md) результатов поиска
* И многое другое

## Общий синтаксис

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

