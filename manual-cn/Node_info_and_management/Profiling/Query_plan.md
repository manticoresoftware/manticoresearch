# 查询计划

<!-- example SHOW PLAN -->

SQL 中的 `SHOW PLAN` 语句和 `"plan": N` 的 JSON 接口选项用于显示查询执行计划。执行计划在实际执行过程中生成并存储，因此对于 SQL 来说，必须在运行该语句**之前**在当前会话中启用分析功能，可以通过 `SET profiling=1` 语句来完成。

在 SQL 模式下，返回两个项目：

- `transformed_tree`：显示全文检索查询的分解结构。
- `enabled_indexes`：提供关于有效辅助索引的信息。

要在 JSON 查询中查看查询执行计划，可以在查询中添加 `"plan": N`。结果将作为 `plan` 属性出现在结果集中。`N` 的取值可以是以下之一：

- 1 - 仅显示根节点的文本计划，类似于 `SHOW PLAN` SQL 查询返回的内容。这是最紧凑的形式。
- 2 - 仅显示 JSON 对象的计划，便于处理。
- 3 - 显示带有每个节点文本描述的 JSON 对象。请注意，子节点的描述也会显示，且会重复部分父节点的描述，这使得整体表示相当庞大。


<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
set profiling=1;

select * from hn_small where match('dog|cat') limit 0;

show plan;
```

<!-- response SQL -->

```sql
*************************** 1. row ***************************
Variable: transformed_tree
   Value: OR(
  AND(KEYWORD(dog, querypos=1)),
  AND(KEYWORD(cat, querypos=2)))
*************************** 2. row ***************************
Variable: enabled_indexes
   Value:
2 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "index": "hn_small",
  "query": {"query_string": "dog|cat"},
  "_source": { "excludes":["*"] },
  "limit": 0,
  "plan": 3
}
```

<!-- response JSON -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 4453,
    "total_relation": "eq",
    "hits": []
  },
  "plan": {
    "query": {
      "type": "OR",
      "description": "OR( AND(KEYWORD(dog, querypos=1)),  AND(KEYWORD(cat, querypos=2)))",
      "children": [
        {
          "type": "AND",
          "description": "AND(KEYWORD(dog, querypos=1))",
          "children": [
            {
              "type": "KEYWORD",
              "word": "dog",
              "querypos": 1
            }
          ]
        },
        {
          "type": "AND",
          "description": "AND(KEYWORD(cat, querypos=2))",
          "children": [
            {
              "type": "KEYWORD",
              "word": "cat",
              "querypos": 2
            }
          ]
        }
      ]
    }
  }
}
```

<!-- end -->

<!-- example SHOW PLAN EXPANSION -->

在某些情况下，由于扩展和其他转换，评估后的查询树可能与原始查询树有很大不同。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SET profiling=1;

SELECT id FROM forum WHERE MATCH('@title way* @content hey') LIMIT 1;

SHOW PLAN;
```

<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)

+--------+
| id     |
+--------+
| 711651 |
+--------+
1 row in set (0.04 sec)

+------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Variable         | Value                                                                                                                                                                                                                                                                                                                                                                                                                   |
+------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| transformed_tree | AND(
  OR(
    OR(
      AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),
      OR(
        AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),
        AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),
    AND(fields=(title), KEYWORD(way, querypos=1, expanded)),
    OR(fields=(title), KEYWORD(way*, querypos=1, expanded))),
  AND(fields=(content), KEYWORD(hey, querypos=2))) |
+------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON全格式：

<!-- request JSON -->

```JSON
POST /search
{
  "index": "forum",
  "query": {"query_string": "@title way* @content hey"},
  "_source": { "excludes":["*"] },
  "limit": 1,
  "plan": 3
}
```

<!-- response JSON -->
```JSON
{
  "took":33,
  "timed_out":false,
  "hits":
  {
    "total":105,
    "hits":
    [
       {
          "_id": 711651,
          "_score":2539,
          "_source":{}
       }
    ]
  },
  "plan":
  {
    "query":
    {
      "type":"AND",
      "description":"AND( OR( OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),  AND(fields=(title), KEYWORD(way, querypos=1, expanded)),  OR(fields=(title), KEYWORD(way*, querypos=1, expanded))),  AND(fields=(content), KEYWORD(hey, querypos=2)))",
      "children":
      [
        {
          "type":"OR",
          "description":"OR( OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),  AND(fields=(title), KEYWORD(way, querypos=1, expanded)),  OR(fields=(title), KEYWORD(way*, querypos=1, expanded)))",
          "children":
          [
            {
               "type":"OR",
               "description":"OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded))))",
               "children":
               [
                 {
                   "type":"AND",
                   "description":"AND(fields=(title), KEYWORD(wayne, querypos=1, expanded))",
                   "fields":["title"],
                   "max_field_pos":0,
                   "children":
                   [
                     {
                       "type":"KEYWORD",
                       "word":"wayne",
                       "querypos":1,
                       "expanded":true
                     }
                   ]
                 },
                 {
                   "type":"OR",
                   "description":"OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))",
                   "children":
                   [
                     {
                       "type":"AND",
                       "description":"AND(fields=(title), KEYWORD(ways, querypos=1, expanded))",
                       "fields":["title"],
                       "max_field_pos":0,
                       "children":
                       [
                         {
                           "type":"KEYWORD",
                           "word":"ways",
                           "querypos":1,
                           "expanded":true
                         }
                       ]
                     },
                     {
                       "type":"AND",
                       "description":"AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded))",
                       "fields":["title"],
                       "max_field_pos":0,
                       "children":
                       [
                         {
                           "type":"KEYWORD",
                           "word":"wayyy",
                           "querypos":1,
                           "expanded":true
                         }
                       ]
                     }
                   ]
                 }
               ]
            },
            {
              "type":"AND",
              "description":"AND(fields=(title), KEYWORD(way, querypos=1, expanded))",
              "fields":["title"],
              "max_field_pos":0,
              "children":
              [
                 {
                    "type":"KEYWORD",
                    "word":"way",
                    "querypos":1,
                    "expanded":true
                 }
              ]
            },
            {
              "type":"OR",
              "description":"OR(fields=(title), KEYWORD(way*, querypos=1, expanded))",
              "fields":["title"],
              "max_field_pos":0,
              "children":
              [
                {
                  "type":"KEYWORD",
                  "word":"way*",
                  "querypos":1,
                  "expanded":true
                }
              ]
            }
          ]
        },
        {
          "type":"AND",
          "description":"AND(fields=(content), KEYWORD(hey, querypos=2))",
          "fields":["content"],
          "max_field_pos":0,
          "children":
          [
            {
              "type":"KEYWORD",
              "word":"hey",
              "querypos":2
            }
          ]
        }
      ]
    }
  }
}
```

<!-- intro -->

##### JSON 对象格式：

<!-- request JSON -->

```JSON
POST /search
{
  "index": "forum",
  "query": {"query_string": "@title way* @content hey"},
  "_source": { "excludes":["*"] },
  "limit": 1,
  "plan": 2
}
```

<!-- response JSON -->

```JSON
{
  "took": 33,
  "timed_out": false,
  "hits": {
    "total": 105,
    "hits": [
      {
        "_id": 711651,
        "_score": 2539,
        "_source": {}
      }
    ]
  },
  "plan": {
    "query": {
      "type": "AND",
      "children": [
        {
          "type": "OR",
          "children": [
            {
              "type": "OR",
              "children": [
                {
                  "type": "AND",
                  "fields": [
                    "title"
                  ],
                  "max_field_pos": 0,
                  "children": [
                    {
                      "type": "KEYWORD",
                      "word": "wayne",
                      "querypos": 1,
                      "expanded": true
                    }
                  ]
                },
                {
                  "type": "OR",
                  "children": [
                    {
                      "type": "AND",
                      "fields": [
                        "title"
                      ],
                      "max_field_pos": 0,
                      "children": [
                        {
                          "type": "KEYWORD",
                          "word": "ways",
                          "querypos": 1,
                          "expanded": true
                        }
                      ]
                    },
                    {
                      "type": "AND",
                      "fields": [
                        "title"
                      ],
                      "max_field_pos": 0,
                      "children": [
                        {
                          "type": "KEYWORD",
                          "word": "wayyy",
                          "querypos": 1,
                          "expanded": true
                        }
                      ]
                    }
                  ]
                }
              ]
            },
            {
              "type": "AND",
              "fields": [
                "title"
              ],
              "max_field_pos": 0,
              "children": [
                {
                  "type": "KEYWORD",
                  "word": "way",
                  "querypos": 1,
                  "expanded": true
                }
              ]
            },
            {
              "type": "OR",
              "fields": [
                "title"
              ],
              "max_field_pos": 0,
              "children": [
                {
                  "type": "KEYWORD",
                  "word": "way*",
                  "querypos": 1,
                  "expanded": true
                }
              ]
            }
          ]
        },
        {
          "type": "AND",
          "fields": [
            "content"
          ],
          "max_field_pos": 0,
          "children": [
            {
              "type": "KEYWORD",
              "word": "hey",
              "querypos": 2
            }
          ]
        }
      ]
    }
  }
}
```

<!-- intro -->

##### JSON 短格式：

<!-- request JSON -->

```JSON
POST /search
{
  "index": "forum",
  "query": {"query_string": "@title way* @content hey"},
  "_source": { "excludes":["*"] },
  "limit": 1,
  "plan": 1
}
```

<!-- response JSON -->

```JSON
{
  "took":33,
  "timed_out":false,
  "hits":
  {
    "total":105,
    "hits":
    [
       {
          "_id": 711651,
          "_score":2539,
          "_source":{}
       }
    ]
  },
  "plan":
  {
    "query":
    {
      "description":"AND( OR( OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),  AND(fields=(title), KEYWORD(way, querypos=1, expanded)),  OR(fields=(title), KEYWORD(way*, querypos=1, expanded))),  AND(fields=(content), KEYWORD(hey, querypos=2)))"
    }
  }
}
```

<!-- end -->

请参见 [EXPLAIN QUERY](../../Searching/Full_text_matching/Profiling.md#不执行查询的情况下进行分析)。它显示全文检索查询的执行树 **而不实际执行查询**。需要注意的是，在对实时表进行查询后使用 `SHOW PLAN` 时，结果将基于随机的磁盘/RAM块。因此，如果您最近修改了表的分词设置，或各个块在词典等方面差异很大，您可能无法得到预期的结果。在这种情况下，建议同时使用 `EXPLAIN QUERY`。

## JSON 结果集说明

`query` 属性包含经过转换的全文检索查询树。每个节点包含：

- `type`：节点类型。可能的值有 `AND`、`OR`、`PHRASE`、`KEYWORD` 等。
- `description`：该节点的查询子树，显示为字符串（以 `SHOW PLAN` 格式）。
- `children`：子节点（如有）。
- `max_field_pos`：字段中的最大位置。
- `word`：转换后的关键字。仅适用于关键字节点。
- `querypos`：该关键字在查询中的位置。仅适用于关键字节点。
- `excluded`：从查询中排除的关键字。仅适用于关键字节点。
- `expanded`：由前缀扩展添加的关键字。仅适用于关键字节点。
- `field_start`：关键字必须出现在字段的最开始处。仅适用于关键字节点。
- `field_end`：关键字必须出现在字段的末尾。仅适用于关键字节点。
- `boost`：关键字的IDF将乘以此值。仅适用于关键字节点。

## SHOW PLAN 的 Dot 格式
`SHOW PLAN format=dot` 允许以层次格式返回全文检索查询执行树，适用于现有工具的可视化，例如 https://dreampuf.github.io/GraphvizOnline。

```sql
MySQL [(none)]> show plan option format=dot\G
*************************** 1. row ***************************
Variable: transformed_tree
   Value: digraph "transformed_tree"
{

0 [shape=record,style=filled,bgcolor="lightgrey" label="AND"]
0 -> 1
1 [shape=record,style=filled,bgcolor="lightgrey" label="AND"]
1 -> 2
2 [shape=record label="i | { querypos=1 }"]
0 -> 3
3 [shape=record,style=filled,bgcolor="lightgrey" label="AND"]
3 -> 4
4 [shape=record label="me | { querypos=2 }"]
}
```

![SHOW PLAN graphviz example](graphviz.png)
<!-- proofread -->
