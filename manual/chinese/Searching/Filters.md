# 过滤器

## WHERE

`WHERE` 是一个 SQL 子句，适用于全文匹配和额外过滤。可用的操作符有：

* [比较操作符](../Searching/Expressions.md#Comparison-operators) `<, >, <=, >=, =, <>, BETWEEN, IN, IS NULL`
* [布尔操作符](../Searching/Full_text_matching/Operators.md#Boolean-operators) `AND, OR, NOT`

`MATCH('query')` 受支持，并映射到 [全文查询](../Searching/Full_text_matching/Operators.md)。

`{col_name | expr_alias} [NOT] IN @uservar` 条件语法受到支持。有关全局用户变量的描述，请参见 [SET](../Server_settings/Setting_variables_online.md#SET) 语法。

## HTTP JSON

如果您更喜欢 HTTP JSON 接口，您也可以应用过滤。这看起来可能比 SQL 更复杂，但在需要以编程方式准备查询的情况下，如用户在您的应用程序中填写表单时，建议使用。

<!-- example json1 -->
这是一个 `bool` 查询中多个过滤器的示例。

这个全文查询匹配所有在任何字段中包含 `product` 的文档。这些文档的价格必须大于或等于 500 (`gte`) 并且小于或等于 1000 (`lte`)。所有这些文档的修订版必须不少于 15 (`lt`)。

<!-- request JSON -->
```json
POST /search
{
  "table": "test1",
  "query": {
    "bool": {
      "must": [
        { "match" : { "_all" : "product" } },
        { "range": { "price": { "gte": 500, "lte": 1000 } } }
      ],
      "must_not": {
        "range": { "revision": { "lt": 15 } }
      }
    }
  }
}
```
<!-- end -->


### bool 查询

<!-- example bool -->
`bool` 查询基于其他查询和/或过滤器的布尔组合匹配文档。查询和过滤器必须在 `must`、`should` 或 `must_not` 部分中指定，并且可以是 [嵌套的](../Searching/Filters.md#Nested-bool-query)。

<!-- request JSON -->
```json
POST /search
{
  "table":"test1",
  "query": {
    "bool": {
      "must": [
        { "match": {"_all":"keyword"} },
        { "range": { "revision": { "gte": 14 } } }
      ]
    }
  }
}
```
<!-- end -->

<!-- example must_not -->
### 必须
在 `must` 部分中指定的查询和过滤器必须匹配文档。如果指定了多个全文查询或过滤器，则所有这些必须匹配。这相当于 SQL 中的 `AND` 查询。请注意，如果您想要匹配一个数组（[多值属性](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)），可以多次指定该属性。只有在数组中找到所有查询值时，结果才会为正，例如：

```json
"must": [
  {"equals" : { "product_codes": 5 }},
  {"equals" : { "product_codes": 6 }}
]
```

另外，请注意，从性能角度考虑，使用：
```json
  {"in" : { "all(product_codes)": [5,6] }}
```
可能会更好（见下文的详细信息）。

### 应该
在 `should` 部分中指定的查询和过滤器应匹配文档。如果在 `must` 或 `must_not` 中指定了一些查询，则 `should` 查询将被忽略。另一方面，如果没有其他查询，除了 `should`，那么至少这些查询中的一个必须匹配文档，文档才能匹配布尔查询。这相当于 `OR` 查询。请注意，如果您想要匹配一个数组（[多值属性](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29），您可以多次指定该属性，例如：

```json
"should": [
  {"equals" : { "product_codes": 7 }},
  {"equals" : { "product_codes": 8 }}
]
```

另外，请注意，从性能角度考虑，使用：
```json
  {"in" : { "any(product_codes)": [7,8] }}
```
可能会更好（见下文的详细信息）。

### must_not
在 `must_not` 部分中指定的查询和过滤器不得匹配文档。如果在 `must_not` 下指定了多个查询，如果没有匹配的文档，则文档匹配。

<!-- request JSON -->
```json
POST /search
{
  "table":"t",
  "query": {
    "bool": {
      "should": [
        {
          "equals": {
            "b": 1
          }
        },
        {
          "equals": {
            "b": 3
          }
        }
      ],
      "must": [
        {
          "equals": {
            "a": 1
          }
        }
      ],
      "must_not": {
        "equals": {
          "b": 2
        }
      }
    }
  }
}
```
<!-- end -->

### 嵌套布尔查询

<!-- example eq_and_or -->
布尔查询可以嵌套在另一个布尔中，因此您可以进行更复杂的查询。要进行嵌套布尔查询，只需使用另一个 `bool` 代替 `must`、`should` 或 `must_not`。以下是如何表示此查询：

```
a = 2 和 (a = 10 或 b = 0)
```

应该在 JSON 中表示。

<!-- request JSON -->
a = 2 和 (a = 10 或 b = 0)

```json
POST /search
{
  "table":"t",
  "query": {
    "bool": {
      "must": [
        {
          "equals": {
            "a": 2
          }
        },
        {
          "bool": {
            "should": [
              {
                "equals": {
                  "a": 10
                }
              },
              {
                "equals": {
                  "b": 0
                }
              }
            ]
          }
        }
      ]
    }
  }
}
```
<!-- end -->

<!-- example complex -->
更复杂的查询：
```
(a = 1 和 b = 1) 或 (a = 10 和 b = 2) 或 (b = 0)
```
<!-- request JSON -->

(a = 1 和 b = 1) 或 (a = 10 和 b = 2) 或 (b = 0)

```json
POST /search
{
  "table":"t",
  "query": {
    "bool": {
      "should": [
        {
          "bool": {
            "must": [
              {
                "equals": {
                  "a": 1
                }
              },
              {
                "equals": {
                  "b": 1
                }
              }
            ]
          }
        },
        {
          "bool": {
            "must": [
              {
                "equals": {
                  "a": 10
                }
              },
              {
                "equals": {
                  "b": 2
                }
              }
            ]
          }
        },
        {
          "bool": {
            "must": [
              {
                "equals": {
                  "b": 0
                }
              }
            ]
          }

        }
      ]
    }
  }
}
```
<!-- end -->

### SQL 格式的查询
<!-- example query_string -->
SQL 格式的查询（`query_string`）也可以在 bool 查询中使用。

<!-- request JSON -->
```json
POST /search
{
  "table": "test1",
  "query": {
    "bool": {
      "must": [
        { "query_string" : "product" },
        { "query_string" : "good" }
      ]
    }
  }
}
```
<!-- end -->

## 各种过滤器

### 相等过滤器
<!-- example equals -->
相等过滤器是最简单的过滤器，适用于整数、浮点数和字符串属性。

<!-- request JSON -->
```json
POST /search
{
  "table":"test1",
  "query": {
    "equals": { "price": 500 }
  }
}
```
<!-- end -->

<!-- example equals_any -->
过滤器 `equals` 可应用于 [multi-value attribute](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) ，并且你可以使用:
* `any()` 如果属性中至少有一个值等于查询值，则匹配；
* `all()` 如果属性只有单个值且该值等于查询值，则匹配

<!-- request JSON -->
```json
POST /search
{
  "table":"test1",
  "query": {
    "equals": { "any(price)": 100 }
  }
}
```
<!-- end -->


### 集合过滤器
<!-- example set -->
集合过滤器检查属性值是否等于指定集合中的任一值。

集合过滤器支持整数、字符串和多值属性。

<!-- request JSON -->
```json
POST /search
{
  "table":"test1",
  "query": {
    "in": {
      "price": [1,10,100]
    }
  }
}
```
<!-- end -->

<!-- example set_any -->
当应用于 [multi-value attribute](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) 时，你可以使用:
* `any()`（等同于无函数），如果属性值与查询值之间至少有一个匹配，则匹配；
* `all()` 如果所有属性值均在查询集合中，则匹配

<!-- request JSON -->
```json
POST /search
{
  "table":"test1",
  "query": {
    "in": {
      "all(price)": [1,10]
    }
  }
}
```
<!-- end -->

### 范围过滤器
<!-- example range -->
范围过滤器匹配属性值在指定范围内的文档。

范围过滤器支持以下属性:
* `gte`: 大于或等于
* `gt`: 大于
* `lte`: 小于或等于
* `lt`: 小于

<!-- request JSON -->
```json
POST /search
{
  "table":"test1",
  "query": {
    "range": {
      "price": {
        "gte": 500,
        "lte": 1000
      }
    }
  }
}
```
<!-- end -->

### 地理距离过滤器

<!-- example geo -->
`geo_distance` 过滤器用于过滤位于特定地理位置一定距离内的文档。

##### location_anchor
指定针脚位置（以度为单位）。距离从该点开始计算。

##### location_source
指定包含纬度和经度的属性。

##### distance_type
指定距离计算函数。可以是 adaptive 或 haversine。adaptive 更快且更精确，详细信息请参见 `GEODIST()`。可选，默认为 adaptive。

##### distance
指定针脚位置的最大距离。在此距离内的所有文档均匹配。距离可以使用各种单位指定。如果未指定单位，则默认以米为单位。以下是支持的距离单位列表:

* 米: `m` 或 `meters`
* 千米: `km` 或 `kilometers`
* 厘米: `cm` 或 `centimeters`
* 毫米: `mm` 或 `millimeters`
* 英里: `mi` 或 `miles`
* 码: `yd` 或 `yards`
* 英尺: `ft` 或 `feet`
* 英寸: `in` 或 `inch`
* 海里: `NM`、`nmi` 或 `nauticalmiles`

`location_anchor` 和 `location_source` 属性接受以下纬度/经度格式:

* 包含 lat 和 lon 键的对象: `{ "lat": "attr_lat", "lon": "attr_lon" }`
* 结构如下的字符串: `"attr_lat, attr_lon"`
* 一个数组，依次为经度和纬度: `[attr_lon, attr_lat]`

纬度和经度均以度为单位.


<!-- request Basic example -->
```json
POST /search
{
  "table":"test",
  "query": {
    "geo_distance": {
      "location_anchor": {"lat":49, "lon":15},
      "location_source": {"attr_lat, attr_lon"},
      "distance_type": "adaptive",
      "distance":"100 km"
    }
  }
}
```

<!-- request Advanced example -->
`geo_distance` 可与匹配或其他属性过滤器一起在 bool 查询中用作过滤器。

```json
POST /search
{
  "table": "geodemo",
  "query": {
    "bool": {
      "must": [
        {
          "match": {
            "*": "station"
          }
        },
        {
          "equals": {
            "state_code": "ENG"
          }
        },
        {
          "geo_distance": {
            "distance_type": "adaptive",
            "location_anchor": {
              "lat": 52.396,
              "lon": -1.774
            },
            "location_source": "latitude_deg,longitude_deg",
            "distance": "10000 m"
          }
        }
      ]
    }
  }
}
```

<!-- end -->
<!-- proofread -->
