# 过滤器

## WHERE

`WHERE` 是一个SQL子句，既适用于全文匹配，也适用于额外的过滤。可用的操作符如下：

* [比较操作符](../Searching/Expressions.md#Comparison-operators) `<, >, <=, >=, =, <>, BETWEEN, IN, IS NULL`
* [布尔操作符](../Searching/Full_text_matching/Operators.md#Boolean-operators) `AND, OR, NOT`

支持 `MATCH('query')`，并对应到[全文查询](../Searching/Full_text_matching/Operators.md)。

支持 `{col_name | expr_alias} [NOT] IN @uservar` 条件语法。有关全局用户变量的说明，请参阅[SET](../Server_settings/Setting_variables_online.md#SET)语法。

## HTTP JSON

如果你更喜欢HTTP JSON接口，也可以应用过滤。它可能看起来比SQL复杂，但推荐用于需要以编程方式准备查询的情况，比如用户在你的应用程序中填写表单时。

<!-- example json1 -->
下面是一个包含多个过滤器的 `bool` 查询示例。

此全文查询匹配所有任何字段包含 `product` 的文档。这些文档的价格必须大于或等于500（`gte`），且小于或等于1000（`lte`）。所有这些文档的修订版本必须不小于15（`lt`）。

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
`bool` 查询基于其他查询和/或过滤的布尔组合匹配文档。查询和过滤器必须在 `must`、`should` 或 `must_not` 部分中指定，且可以[嵌套](../Searching/Filters.md#Nested-bool-query)。

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
### must
在 `must` 部分指定的查询和过滤器是必须匹配的文档。如果指定了多个全文查询或过滤器，所有这些都必须匹配。这相当于SQL中的 `AND` 查询。注意，如果你想对数组（[多值属性](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)）进行匹配，可以多次指定该属性。仅当数组中找到所有查询值时，结果才为正，例如：

```json
"must": [
  {"equals" : { "product_codes": 5 }},
  {"equals" : { "product_codes": 6 }}
]
```

另请注意，性能方面可能更好的是使用：
```json
  {"in" : { "all(product_codes)": [5,6] }}
```
（详见下文）。

### should
在 `should` 部分指定的查询和过滤器应匹配文档。如果在 `must` 或 `must_not` 中已指定查询，则忽略 `should` 查询。另一方面，如果除了 `should` 之外没有其他查询，则这些查询中的至少一个必须匹配文档，文档才能匹配 `bool` 查询。这相当于 `OR` 查询。注意，如果你想对数组（[多值属性](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)）进行匹配，可以多次指定该属性，例如：

```json
"should": [
  {"equals" : { "product_codes": 7 }},
  {"equals" : { "product_codes": 8 }}
]
```

另请注意，性能方面可能更好的是使用：
```json
  {"in" : { "any(product_codes)": [7,8] }}
```
（详见下文）。

### must_not
在 `must_not` 部分指定的查询和过滤器必须不匹配文档。如果在 `must_not` 下指定多个查询，则只要没有一个匹配，文档即匹配。

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

### 嵌套 bool 查询

<!-- example eq_and_or -->
一个 `bool` 查询可以嵌套在另一个 `bool` 中，以构造更复杂的查询。要创建嵌套的布尔查询，只需用另一个 `bool` 代替 `must`、`should` 或 `must_not`。下面是这样一个查询：

```
a = 2 and (a = 10 or b = 0)
```

在JSON中的表现形式。

<!-- request JSON -->
a = 2 and (a = 10 or b = 0)

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
(a = 1 and b = 1) or (a = 10 and b = 2) or (b = 0)
```
<!-- request JSON -->

(a = 1 and b = 1) or (a = 10 and b = 2) or (b = 0)

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

### SQL格式查询
<!-- example query_string -->
SQL格式的查询（`query_string`）也可用在 `bool` 查询中。

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

## 各类过滤器

### 等值过滤器
<!-- example equals -->
等值过滤器是最简单的过滤器，可应用于整数、浮点数和字符串属性。

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
`equals` 过滤器也可以应用于[多值属性](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)，你可以使用：
* `any()`：如果属性至少有一个值等于查询值，则为真；
* `all()`：如果属性只有一个值且等于查询值，则为真

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
应用于[多值属性](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)时，可以使用：
* `any()`（等同于无函数）：只要属性值与查询值集中至少存在一个匹配即为真；
* `all()`：只有当属性的所有值都包含在查询值集合中时，才为真

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

范围过滤器支持以下属性：
* `gte`：大于或等于
* `gt`：大于
* `lte`：小于或等于
* `lt`：小于

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
`geo_distance` 过滤器用于过滤位于指定地理位置特定距离范围内的文档。

##### location_anchor
指定定位点位置，单位为度。距离从该点计算。

##### location_source
指定包含纬度和经度的属性。

##### distance_type
指定距离计算函数。可以是 adaptive 或 haversine。adaptive 更快且更精确，更多详情请参见 `GEODIST()`。可选，默认值为 adaptive。

##### distance
指定距离针脚位置的最大距离。所有在此距离范围内的文档均匹配。距离可以用各种单位指定。如果未指定单位，则距离默认为米。以下是支持的距离单位列表：

* 米：`m` 或 `meters`
* 千米：`km` 或 `kilometers`
* 厘米：`cm` 或 `centimeters`
* 毫米：`mm` 或 `millimeters`
* 英里：`mi` 或 `miles`
* 码：`yd` 或 `yards`
* 英尺：`ft` 或 `feet`
* 英寸：`in` 或 `inch`
* 海里：`NM`，`nmi` 或 `nauticalmiles`

`location_anchor` 和 `location_source` 属性接受以下纬度/经度格式：

* 具有 lat 和 lon 键的对象： `{ "lat": "attr_lat", "lon": "attr_lon" }`
* 以下结构的字符串： `"attr_lat, attr_lon"`
* 按以下顺序排列的纬度和经度数组： `[attr_lon, attr_lat]`

纬度和经度以度为单位。


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
`geo_distance` 可以作为 bool 查询中的过滤器，与匹配或其他属性过滤器一起使用。

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

