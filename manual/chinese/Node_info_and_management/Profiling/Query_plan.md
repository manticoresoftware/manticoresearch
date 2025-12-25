# 查询计划

<!-- example SHOW PLAN -->

`SHOW PLAN` SQL 语句和 `"plan": N` JSON 接口选项显示查询执行计划。该计划在实际执行过程中生成并存储，因此对于 SQL 而言，在运行该语句之前，当前会话必须启用剖析功能。这可以通过执行 `SET profiling=1` 语句来完成。

SQL 模式下返回两个项目：
* `transformed_tree`，显示完整的查询分解。
* `enabled_indexes`，提供关于有效二级索引的信息。

要在 JSON 查询中查看查询执行计划，请在查询中添加 `"plan": N`。结果将在结果集中以 `plan` 属性的形式出现。`N` 可以是以下之一：
* 1 - 仅显示根节点的文本计划，类似于 `SHOW PLAN` SQL 查询返回的内容。这是最紧凑的形式。
* 2 - 仅显示 JSON 对象计划，适用于处理。
* 3 - 显示一个 JSON 对象，其中包含每个节点的文本描述。注意，子节点的描述也存在，并重复了父节点的部分描述，从而使整个表示非常庞大。


<!-- intro -->
##### SQL：
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
##### JSON：

<!-- request JSON -->

```json
POST /search
{
  "table": "hn_small",
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

有时，评估后的查询树可能与原始树有很大不同，由于扩展和其他转换。

<!-- intro -->
##### SQL：
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
##### JSON 全格式：

<!-- request JSON -->

```JSON
POST /search
{
  "table": "forum",
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
  "table": "forum",
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
  "table": "forum",
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

参见 [EXPLAIN QUERY](../../Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query)。它显示全文本查询的执行树 **而不实际执行查询**。请注意，使用 `SHOW PLAN` 查看实时表后的查询结果将基于随机磁盘/RAM 块。因此，如果您最近修改了表的分词设置，或者块在字典等方面差异很大，您可能不会得到预期的结果。请考虑同时使用 `EXPLAIN QUERY`。

## JSON 结果集注释

`query` 属性包含转换后的全文本查询树。每个节点包含：

* `type`：节点类型。可以是 `AND`、`OR`、`PHRASE`、`KEYWORD` 等。
* `description`：此节点的查询子树以字符串形式（类似于 `SHOW PLAN` 格式）显示。
* `children`：如果有，则为子节点。
* `max_field_pos`：字段内的最大位置。
* `word`：转换后的关键词。关键词节点专用。
* `querypos`：此关键词在查询中的位置。关键词节点专用。
* `excluded`：从查询中排除的关键词。关键词节点专用。
* `expanded`：通过前缀扩展添加的关键词。关键词节点专用。
* `field_start`：关键词必须出现在字段的开头。关键词节点专用。
* `field_end`：关键词必须出现在字段的结尾。关键词节点专用。
* `boost`：关键词 IDF 将乘以此值。关键词节点专用。

## SHOW PLAN 的点格式
`SHOW PLAN format=dot` 允许以适合现有工具（如 https://dreampuf.github.io/GraphvizOnline）可视化的层次结构格式返回全文本查询执行树：

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

![SHOW PLAN graphviz 示例](graphviz.png)
<!-- proofread -->

