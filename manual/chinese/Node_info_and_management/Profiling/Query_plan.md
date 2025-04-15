# 查询计划

<!-- 示例 SHOW PLAN -->

`SHOW PLAN` SQL 语句以及 `"plan": N` JSON 接口选项显示查询执行计划。该计划在实际执行期间生成并存储，因此在 SQL 的情况下，必须在当前会话中 **在** 运行该语句之前启用 Profiling。这可以通过 `SET profiling=1` 语句完成。

在 SQL 模式下返回两个项目：
* `transformed_tree`，显示全文查询分解。
* `enabled_indexes`，提供有效的次级索引的信息。

要在 JSON 查询中查看查询执行计划，请向查询添加 `"plan": N`。结果将作为结果集中的 `plan` 属性出现。 `N` 可以是以下之一：
* 1 - 仅显示根节点的文本计划，类似于在 `SHOW PLAN` SQL 查询中返回的内容。这是最紧凑的形式。
* 2 - 仅显示 JSON 对象计划，适合处理。
* 3 - 显示包含每个节点文本描述的 JSON 对象。请注意，子节点的描述也存在，并且重复了部分父节点的描述，这使得整个表示相当庞大。


<!-- 介绍 -->
##### SQL:
<!-- 请求 SQL -->

```sql
set profiling=1;

select * from hn_small where match('dog|cat') limit 0;

show plan;
```

<!-- 响应 SQL -->

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

<!-- 介绍 -->
##### JSON:

<!-- 请求 JSON -->

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

<!-- 响应 JSON -->
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

<!-- 结束 -->

<!-- 示例 SHOW PLAN 扩展 -->

在某些情况下，由于扩展和其他变换，评估的查询树可能与原始查询相差甚远。

<!-- 介绍 -->
##### SQL:
<!-- 请求 SQL -->

```sql
SET profiling=1;

SELECT id FROM forum WHERE MATCH('@title way* @content hey') LIMIT 1;

SHOW PLAN;
```

<!-- 响应 SQL -->

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

<!-- 介绍 -->
##### JSON 完整格式：

<!-- 请求 JSON -->

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

<!-- 响应 JSON -->
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

##### JSON对象格式:

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
              "word": "嘿",
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

##### JSON简短格式:

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

另见 [EXPLAIN QUERY](../../Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query)。它显示了一个全文查询的执行树 **而不实际执行查询**。 请注意，在对实时表使用 `SHOW PLAN` 进行查询时，结果将基于随机磁盘/RAM 块。因此，如果您最近修改了表的标记设置，或者如果块在字典等方面有显著差异，您可能不会获得所期望的结果。 请考虑这一点，并考虑使用 `EXPLAIN QUERY`。

## JSON结果集说明

`query` 属性包含转换后的全文查询树。每个节点包含:

* `type`: 节点类型。可以是 `AND`，`OR`，`PHRASE`，`KEYWORD` 等。
* `description`: 显示为字符串的此节点的查询子树（以 `SHOW PLAN` 格式）。
* `children`: 子节点（如果有）。
* `max_field_pos`: 字段内的最大位置。
* `word`: 转换后的关键词。仅限关键词节点。
* `querypos`: 此关键词在查询中的位置。仅限关键词节点。
* `excluded`: 从查询中排除的关键词。仅限关键词节点。
* `expanded`: 通过前缀扩展添加的关键词。仅限关键词节点。
* `field_start`: 关键词必须出现在字段的开始。仅限关键词节点。
* `field_end`: 关键词必须出现在字段的末尾。仅限关键词节点。
* `boost`: 关键词 IDF 将乘以此值。仅限关键词节点。

## SHOW PLAN 的点格式
`SHOW PLAN format=dot` 允许以适合现有工具可视化的层次格式返回全文查询执行树，例如 https://dreampuf.github.io/GraphvizOnline：

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
