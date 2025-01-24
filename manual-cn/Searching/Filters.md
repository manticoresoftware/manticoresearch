# 过滤器

## WHERE

`WHERE` 是一个 SQL 子句，用于全文本匹配和附加过滤。支持以下操作符：

- [比较操作符](../Searching/Expressions.md#比较运算符) `<, >, <=, >=, =, <>, BETWEEN, IN, IS NULL`
- [布尔操作符](../Searching/Full_text_matching/Operators.md#布尔操作符) `AND, OR, NOT`

支持 `MATCH('query')`，它映射到 [全文查询](../Searching/Full_text_matching/Operators.md)。

支持 `{col_name | expr_alias} [NOT] IN @uservar` 条件语法。请参阅 [SET](../Server_settings/Setting_variables_online.md#SET) 语法了解全局用户变量的描述。

## HTTP JSON

如果您更喜欢使用 HTTP JSON 接口，也可以进行过滤。虽然看起来比 SQL 更复杂，但在需要编程生成查询时，推荐使用这种方法，比如当用户在您的应用中填写表单时。

<!-- example json1 -->
以下是 `bool` 查询中几个过滤器的示例。

这个全文查询匹配在任何字段中包含`product`的所有文档。这些文档的价格必须大于或等于 500（`gte`）且小于或等于 1000（`lte`）。所有这些文档的修订版本都不能小于 15（`lt`）。

<!-- request JSON -->

```json
POST /search
{
  "index": "test1",
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
`bool` 查询基于其他查询和/或过滤器的布尔组合匹配文档。查询和过滤器必须在 `must`、`should` 或 `must_not` 部分中指定，并且可以[嵌套](../Searching/Filters.md#嵌套布尔查询)。

<!-- request JSON -->

```json
POST /search
{
  "index":"test1",
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
### must
在 `must` 部分中指定的查询和过滤器必须匹配文档。如果指定了多个全文查询或过滤器，它们必须全部匹配。这等同于 SQL 中的 `AND` 查询。请注意，如果要匹配数组（[多值属性](../Creating_a_table/Data_types.md#多值整数-%28Multi-value-integer-、MVA%29)），可以多次指定该属性。只有在数组中找到所有查询的值，结果才会为正，例如：

```json
"must": [
  {"equals" : { "product_codes": 5 }},
  {"equals" : { "product_codes": 6 }}
]
```

另外，出于性能考虑，可能更适合使用：
```json
  {"in" : { "all(product_codes)": [5,6] }}
```
（见下文详细说明）。

### should
在 `should` 部分中指定的查询和过滤器应匹配文档。如果在 `must` 或 `must_not` 中指定了一些查询，则会忽略 `should` 查询。另一方面，如果除了 `should` 没有其他查询，那么至少其中一个查询必须匹配文档，文档才能匹配该 bool 查询。这等同于 SQL 中的 `OR` 查询。请注意，如果要匹配数组（[多值属性](../Creating_a_table/Data_types.md#多值整数（Multi-value integer 、MVA）)⛔），可以多次指定该属性，例如：

```json
"should": [
  {"equals" : { "product_codes": 7 }},
  {"equals" : { "product_codes": 8 }}
]
```

另外，出于性能考虑，可能更适合使用：
```json
  {"in" : { "any(product_codes)": [7,8] }}
```
（见下文详细说明）。

### must_not
在 `must_not` 部分中指定的查询和过滤器不能匹配文档。如果在 `must_not` 下指定了多个查询，文档如果都不匹配这些查询，则视为匹配。

<!-- request JSON -->
```json
POST /search
{
  "index":"t",
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
一个布尔查询可以嵌套在另一个布尔查询中，从而构建更复杂的查询。要创建嵌套的布尔查询，只需在 `must`、`should` 或 `must_not` 中使用另一个 `bool`。以下是如何将此查询：

```
a = 2 and (a = 10 or b = 0)
```

表示为JSON格式。

<!-- request JSON -->
a = 2 and (a = 10 or b = 0)

```json
POST /search
{
  "index":"t",
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
(a = 1 and b = 1) or (a = 10 and b = 2) or (b = 0)
```
<!-- request JSON -->

(a = 1 and b = 1) or (a = 10 and b = 2) or (b = 0)

```json
POST /search
{
  "index":"t",
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

### SQL格式的查询
<!-- example query_string -->
SQL格式的查询（`query_string`）也可以用于布尔查询中。

<!-- request JSON -->
```json
POST /search
{
  "index": "test1",
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

### 等式过滤器
<!-- example equals -->
等式过滤器是最简单的过滤器，适用于整数、浮点数和字符串属性。

<!-- request JSON -->
```json
POST /search
{
  "index":"test1",
  "query": {
    "equals": { "price": 500 }
  }
}
```
<!-- end -->

<!-- example equals_any -->

`equals` 过滤器可以应用于[多值属性](../Creating_a_table/Data_types.md#多值整数（Multi-value integer 、MVA）)，你可以使用：

- `any()`，当属性中至少有一个值等于查询值时，结果为正；
- `all()`，当属性只有一个值且它等于查询值时，结果为正。

<!-- request JSON -->
```json
POST /search
{
  "index":"test1",
  "query": {
    "equals": { "any(price)": 100 }
  }
}
```
<!-- end -->


### 集合过滤器
<!-- example set -->

集合过滤器检查属性值是否等于指定集合中的任意一个值。

集合过滤器支持整数、字符串和多值属性。

<!-- request JSON -->
```json
POST /search
{
  "index":"test1",
  "query": {
    "in": {
      "price": [1,10,100]
    }
  }
}
```
<!-- end -->

<!-- example set_any -->

当应用于[多值属性](../Creating_a_table/Data_types.md#多值整数（Multi-value integer 、MVA）)时，你可以使用：

- `any()`（等同于不使用函数），当属性值与查询值集中的至少一个匹配时，结果为正；
- `all()`，当所有属性值都在查询值集中时，结果为正。

<!-- request JSON -->
```json
POST /search
{
  "index":"test1",
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

范围过滤器匹配具有指定范围内属性值的文档。

范围过滤器支持以下属性：

- `gte`：大于或等于
- `gt`：大于
- `lte`：小于或等于
- `lt`：小于

<!-- request JSON -->
```json
POST /search
{
  "index":"test1",
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
`geo_distance` 过滤器用于筛选距离指定地理位置一定范围内的文档。

##### location_anchor
指定锚点位置，单位为度。距离是从这个点计算的。

##### location_source
指定包含纬度和经度的属性。

##### distance_type
指定距离计算函数。可以是 adaptive 或 haversine。adaptive 更快且更精确，详情请参阅 `GEODIST()`。此选项为可选项，默认为 adaptive。

##### distance
指定与锚点位置的最大距离。在此距离内的所有文档都会匹配。距离可以以多种单位指定。如果没有指定单位，默认以米为单位。以下是支持的距离单位列表：

- 米：`m` 或 `meters`
- 千米：`km` 或 `kilometers`
- 厘米：`cm` 或 `centimeters`
- 毫米：`mm` 或 `millimeters`
- 英里：`mi` 或 `miles`
- 码：`yd` 或 `yards`
- 英尺：`ft` 或 `feet`
- 英寸：`in` 或 `inch`
- 海里：`NM`，`nmi` 或 `nauticalmiles`

`location_anchor` 和 `location_source` 属性接受以下纬度/经度格式：

- 带有 lat 和 lon 键的对象：`{ "lat": "attr_lat", "lon": "attr_lon" }`
- 具有以下结构的字符串：`"attr_lat, attr_lon"`
- 以经度和纬度为顺序的数组：`[attr_lon, attr_lat]`

纬度和经度的单位为度。


<!-- request Basic example -->
```json
POST /search
{
  "index":"test",
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
`geo_distance` 可以与布尔查询中的匹配查询或其他属性过滤器一起用作过滤器。

```json
POST /search
{
  "index": "geodemo",
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