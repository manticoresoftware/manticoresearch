# Introduction into searching with Manticore Search

Searching is a core feature of Manticore Search. You can:
* Perform [full-text search](../Searching/Full_text_matching/Basic_usage.md#MATCH) and implement search result [highlighting](../Searching/Highlighting.md)
* Apply [non-full-text filtering](../Searching/Filters.md)
* Use [expressions](../Searching/Expressions.md) for filtering
* Utilize various [search options](../Searching/Options.md)
* Employ [multi-queries](../Searching/Multi-queries.md) and [sub-selects](../Searching/Sub-selects.md)
* Conduct [aggregations](../Searching/Grouping.md) and [faceting](../Searching/Faceted_search.md) of search results
* And much more

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
<!-- proofread -->
