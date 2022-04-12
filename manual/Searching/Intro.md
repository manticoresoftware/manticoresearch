# Introduction into searching with Manticore Search

Search is a key functionality of Manticore Search. With Manticore you can:
* Do [full-text search](../Searching/Full_text_matching/Basic_usage.md#MATCH) and [highlight](../Searching/Highlighting.md) matching text.
* [filter](../Searching/Filters.md) by attributes.
* Use [expressions](../Searching/Expressions.md).
* Use numerous [search options](../Searching/Options.md).
* Use [multi-queries](../Searching/Multi-queries.md) and [sub-selects](../Searching/Sub-selects.md).
* Do [grouping](../Searching/Grouping.md) and [faceting](../Searching/Faceted_search.md) of search results.
* And many more.

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
