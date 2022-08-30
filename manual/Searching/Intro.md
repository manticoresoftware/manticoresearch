# Introduction into searching with Manticore Search

Search is a key functionality of Manticore Search. You can:
* Do [full-text search](../Searching/Full_text_matching/Basic_usage.md#MATCH) and search results [highlighting](../Searching/Highlighting.md)
* Do [non-full-text filtering](../Searching/Filters.md)
* Use [expressions](../Searching/Expressions.md) for filtering
* Use various [search options](../Searching/Options.md)
* Use [multi-queries](../Searching/Multi-queries.md) and [sub-selects](../Searching/Sub-selects.md)
* Do [aggreations](../Searching/Grouping.md) and [faceting](../Searching/Faceted_search.md) of search results
* And many more

## General syntax

**SQL**:
```sql
SELECT ... [OPTION <optionname>=<value> [ , ... ]]
```

**HTTP**:
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
