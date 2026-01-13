# 过滤器

## WHERE

`WHERE` 是一个 SQL 子句，适用于全文匹配和额外的过滤。以下操作符可用：

* [比较操作符](../Searching/Expressions.md#Comparison-operators) `<, >, <=, >=, =, <>, BETWEEN, IN, IS NULL`
* [布尔操作符](../Searching/Full_text_matching/Operators.md#Boolean-operators) `AND, OR, NOT`

`MATCH('query')` 受支持，并映射到一个 [全文查询](../Searching/Full_text_matching/Operators.md)。

`{col_name | expr_alias} [NOT] IN @uservar` 条件语法受支持。请参阅 [SET](../Server_settings/Setting_variables_online.md#SET) 语法以了解全局用户变量的描述。

## HTTP JSON

如果您更喜欢 HTTP JSON 接口，也可以应用过滤。它可能看起来比 SQL 更复杂，但在需要程序化准备查询的情况下（例如，当用户在您的应用程序中填写表单时），它被推荐使用。

<!-- example json1 -->
以下是一个 `bool` 查询中几个过滤器的示例。

此全文查询匹配所有包含 `product` 的字段的文档。这些文档必须具有价格大于或等于 500（`gte`）且小于或等于 1000（`lte`）。所有这些文档都必须没有小于 15 的修订版本（`lt`）。

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
`bool` 查询根据其他查询和/或过滤器的布尔组合匹配文档。查询和过滤器必须在 `must`、`should` 或 `must_not` 部分指定，并且可以 [嵌套](../Searching/Filters.md#Nested-bool-query)。

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
`must` 部分中指定的查询和过滤器必须匹配文档。如果指定了多个全文查询或过滤器，则所有这些查询都必须匹配。这相当于 SQL 中的 `AND` 查询。请注意，如果您要匹配数组（[多值属性](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)），可以多次指定该属性。结果仅在数组中找到所有查询值时为正，例如：

```json
"must": [
  {"equals" : { "product_codes": 5 }},
  {"equals" : { "product_codes": 6 }}
]
```

另外，从性能角度来看，使用：
```json
  {"in" : { "all(product_codes)": [5,6] }}
```
（请参阅下方的详细信息）可能更好。

### should
`should` 部分中指定的查询和过滤器应该匹配文档。如果在 `must` 或 `must_not` 中指定了某些查询，则 `should` 查询将被忽略。另一方面，如果除了 `should` 之外没有其他查询，则至少有一个这些查询必须匹配一个文档，该文档才能匹配 `bool` 查询。这相当于 `OR` 查询。请注意，如果您要匹配数组（[多值属性](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)），可以多次指定该属性，例如：

```json
"should": [
  {"equals" : { "product_codes": 7 }},
  {"equals" : { "product_codes": 8 }}
]
```

另外，从性能角度来看，使用：
```json
  {"in" : { "any(product_codes)": [7,8] }}
```
（请参阅下方的详细信息）可能更好。

### must_not
`must_not` 部分中指定的查询和过滤器必须不匹配文档。如果在 `must_not` 下指定了多个查询，则文档匹配如果它们中的任何一个都不匹配。

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
`bool` 查询可以嵌套在另一个 `bool` 中，因此您可以创建更复杂的查询。要创建嵌套布尔查询，请使用另一个 `bool` 而不是 `must`、`should` 或 `must_not`。以下是如何表示此查询：

```
a = 2 and (a = 10 or b = 0)
```

以 JSON 格式呈现。

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

### SQL 格式的查询
<!-- example query_string -->
SQL 格式的查询（`query_string`）也可以在 `bool` 查询中使用。

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

### 等值过滤器
<!-- example equals -->
等值过滤器是最简单的过滤器，适用于整数、浮点数和字符串属性。

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
`equals` 过滤器可以应用于 [多值属性](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)，您可以使用：
* `any()`，如果属性至少有一个值等于查询值，则为正；
* `all()`，如果属性只有一个值且等于查询值，则为正

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
集合过滤器检查属性值是否等于指定集合中的任何值。

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
当应用于 [多值属性](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) 时，您可以使用：
* `any()`（等同于无函数），如果属性值与查询值之间至少有一个匹配，则为正；
* `all()`，如果所有属性值都在查询集中，则为正

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
范围过滤器匹配具有属性值在指定范围内的文档。

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
`geo_distance` 过滤器用于过滤距离特定地理位置特定距离内的文档。

##### location_anchor
指定针的位置，以度为单位。距离从这一点计算。

##### location_source
指定包含纬度和经度的属性。

##### distance_type
指定距离计算函数。可以是 adaptive 或 haversine。adaptive 更快且更精确，有关详细信息，请参阅 `GEODIST()`。可选，默认为 adaptive。

##### 距离
指定从针位置的最大距离。在此距离内的所有文档都匹配。距离可以使用多种单位指定。如果没有指定单位，则距离默认为米。以下是支持的距离单位列表：

* 米: `m` 或 `meters`
* 公里: `km` 或 `kilometers`
* 厘米: `cm` 或 `centimeters`
* 毫米: `mm` 或 `millimeters`
* 英里: `mi` 或 `miles`
* 码: `yd` 或 `yards`
* 英尺: `ft` 或 `feet`
* 英寸: `in` 或 `inch`
* 海里: `NM`, `nmi` 或 `nauticalmiles`

`location_anchor` 和 `location_source` 属性接受以下纬度/经度格式：

* 具有 lat 和 lon 键的对象: `{ "lat": "attr_lat", "lon": "attr_lon" }`
* 具有以下结构的字符串: `"attr_lat, attr_lon"`
* 按以下顺序排列的数组: `[attr_lon, attr_lat]`

纬度和经度以度为单位指定。


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
`geo_distance` 可以作为 bool 查询中的过滤器与其他匹配项或其它属性过滤器一起使用。

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

