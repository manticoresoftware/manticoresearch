# Manticore Search के साथ खोजने में परिचय

खोज Manticore Search की एक मूल विशेषता है। आप कर सकते हैं:
* [पूर्ण-टेक्स्ट खोज](../Searching/Full_text_matching/Basic_usage.md#MATCH) करें और खोज परिणामों के लिए [हाइलाइटिंग](../Searching/Highlighting.md) लागू करें
* [k-निकटतम पड़ोसी खोज](../Searching/KNN.md) करें
* [गैर-पूर्ण-टेक्स्ट फ़िल्टरिंग](../Searching/Filters.md) लागू करें
* फ़िल्टरिंग के लिए [व्यक्तियों](../Searching/Expressions.md) का उपयोग करें
* विभिन्न [खोज विकल्पों](../Searching/Options.md) का उपयोग करें
* [बहु-प्रश्नों](../Searching/Multi-queries.md) और [उप-चयन](../Searching/Sub-selects.md) का उपयोग करें
* खोज परिणामों की [संग्रह](../Searching/Grouping.md) और [फैसिंग](../Searching/Faceted_search.md) करें
* और भी बहुत कुछ

## सामान्य सिंटैक्स

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
