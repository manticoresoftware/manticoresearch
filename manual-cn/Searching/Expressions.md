# 搜索中的表达式

Manticore 允许通过 SQL 和 HTTP 使用任意算术表达式，这些表达式可以结合属性值、内部属性（文档 ID 和相关性权重）、算术操作、多个内置函数以及用户定义的函数。以下是完整的参考列表，方便快速查阅。

## 算术运算符
```sql
+, -, *, /, %, DIV, MOD
```

提供标准的算术运算符。涉及这些运算符的算术计算可以在三种模式下执行：

1. 使用单精度 32 位 IEEE 754 浮点值（默认模式），
2. 使用带符号的 32 位整数，
3. 使用 64 位带符号整数。

如果没有操作结果为浮点值，表达式解析器会自动切换到整数模式。否则，将使用默认的浮点模式。例如，如果 `a` 和 `b` 都是 32 位整数，那么 `a+b` 将使用 32 位整数计算；如果其中一个是 64 位整数，则使用 64 位整数计算；否则使用浮点数计算。然而，`a/b` 或 `sqrt(a)` 将始终以浮点数计算，因为这些操作返回非整数结果。要避免这种情况，您可以使用 `IDIV(a,b)` 或 `a DIV b`。此外，`a*b` 不会自动提升为 64 位结果。要强制得到 64 位结果，可以使用 [BIGINT()](../Functions/Type_casting_functions.md#BIGINT())，但请注意，如果存在非整数操作，`BIGINT()` 将被忽略。

## 比较运算符
```sql
<, > <=, >=, =, <>
```

比较运算符在条件为真时返回 `1.0`，否则返回 `0.0`。例如，`(a=b)+3` 在属性 `a` 等于属性 `b` 时，返回 4；如果 `a` 不等于 `b`，则返回 3。与 MySQL 不同，等值比较（即 `=` 和 `<>` 运算符）包括一个小的等值阈值（默认为 1e-6）。如果比较值之间的差异在该阈值内，它们将被视为相等。

`BETWEEN` 和 `IN` 运算符在多值属性的情况下，如果至少有一个值匹配条件，则返回 true（类似于 [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY())）。`IN` 运算符不支持 JSON 属性。`IS (NOT) NULL` 运算符仅支持 JSON 属性。

## 布尔运算符
```sql
AND, OR, NOT
```

布尔运算符（AND、OR、NOT）的行为如预期。它们是左结合的，并且优先级低于其他运算符。NOT 的优先级高于 AND 和 OR，但仍然低于任何其他运算符。AND 和 OR 的优先级相同，因此在复杂表达式中建议使用括号以避免混淆。

## 位运算符
```sql
&, |
```

这些运算符分别执行按位与和按位或。操作数必须是整数类型。

## 函数：
* [ABS()](../Functions/Mathematical_functions.md#ABS%28%29)
* [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29)
* [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)
* [ATAN2()](../Functions/Mathematical_functions.md#ATAN2%28%29)
* [BIGINT()](../Functions/Type_casting_functions.md#BIGINT%28%29)
* [BITDOT()](../Functions/Mathematical_functions.md#BITDOT%28%29)
* [BM25F()](../Functions/Searching_and_ranking_functions.md#BM25F%28%29)
* [CEIL()](../Functions/Mathematical_functions.md#CEIL%28%29)
* [CONCAT()](../Functions/String_functions.md#CONCAT%28%29)
* [CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29)
* [COS()](../Functions/Mathematical_functions.md#COS%28%29)
* [CRC32()](../Functions/Mathematical_functions.md#CRC32%28%29)
* [DATE_HISTOGRAM()](../Functions/Date_and_time_functions.md#DATE_HISTOGRAM%28%29)
* [DATE_RANGE()](../Functions/Date_and_time_functions.md#DATE_RANGE%28%29)
* [DAY()](../Functions/Date_and_time_functions.md#DAY%28%29)
* [DOUBLE()](../Functions/Type_casting_functions.md#DOUBLE%28%29)
* [EXP()](../Functions/Mathematical_functions.md#EXP%28%29)
* [FIBONACCI()](../Functions/Mathematical_functions.md#FIBONACCI%28%29)
* [FLOOR()](../Functions/Mathematical_functions.md#FLOOR%28%29)
* [GEODIST()](../Functions/Geo_spatial_functions.md#GEODIST%28%29)
* [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29)
* [GREATEST()](../Functions/Mathematical_functions.md#GREATEST%28%29)
* [HOUR()](../Functions/Date_and_time_functions.md#HOUR%28%29)
* [HISTOGRAM()](../Functions/Arrays_and_conditions_functions.md#HISTOGRAM%28%29)
* [IDIV()](../Functions/Mathematical_functions.md#IDIV%28%29)
* [IF()](../Functions/Arrays_and_conditions_functions.md#IF%28%29)
* [IN()](../Functions/Arrays_and_conditions_functions.md#IN%28%29)
* [INDEXOF()](../Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29)
* [INTEGER()](../Functions/Type_casting_functions.md#INTEGER%28%29)
* [INTERVAL()](../Functions/Arrays_and_conditions_functions.md#INTERVAL%28%29)
* [LAST_INSERT_ID()](../Functions/Other_functions.md#LAST_INSERT_ID%28%29)
* [LEAST()](../Functions/Mathematical_functions.md#LEAST%28%29)
* [LENGTH()](../Functions/Arrays_and_conditions_functions.md#LENGTH%28%29)
* [LN()](../Functions/Mathematical_functions.md#LN%28%29)
* [LOG10()](../Functions/Mathematical_functions.md#LOG10%28%29)
* [LOG2()](../Functions/Mathematical_functions.md#LOG2%28%29)
* [MAX()](../Functions/Mathematical_functions.md#MAX%28%29)
* [MIN()](../Functions/Mathematical_functions.md#MIN%28%29)
* [MINUTE()](../Functions/Date_and_time_functions.md#MINUTE%28%29)
* [MIN_TOP_SORTVAL()](../Functions/Searching_and_ranking_functions.md#MIN_TOP_SORTVAL%28%29)
* [MIN_TOP_WEIGHT()](../Functions/Searching_and_ranking_functions.md#MIN_TOP_WEIGHT%28%29)
* [MONTH()](../Functions/Date_and_time_functions.md#MONTH%28%29)
* [NOW()](../Functions/Date_and_time_functions.md#NOW%28%29)
* [PACKEDFACTORS()](../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29)
* [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29)
* [POW()](../Functions/Mathematical_functions.md#POW%28%29)
* [RAND()](../Functions/Mathematical_functions.md#RAND%28%29)
* [RANGE()](../Functions/Arrays_and_conditions_functions.md#RANGE%28%29)
* [REGEX()](../Functions/String_functions.md#REGEX%28%29)
* [REMAP()](../Functions/Arrays_and_conditions_functions.md#REMAP%28%29)
* [SECOND()](../Functions/Date_and_time_functions.md#SECOND%28%29)
* [SIN()](../Functions/Mathematical_functions.md#SIN%28%29)
* [SINT()](../Functions/Type_casting_functions.md#SINT%28%29)
* [SQRT()](../Functions/Mathematical_functions.md#SQRT%28%29)
* [SUBSTRING_INDEX()](../Functions/String_functions.md#SUBSTRING_INDEX%28%29)
* [TO_STRING()](../Functions/Type_casting_functions.md#TO_STRING%28%29)
* [UINT()](../Functions/Type_casting_functions.md#UINT%28%29)
* [YEAR()](../Functions/Date_and_time_functions.md#YEAR%28%29)
* [YEARMONTH()](../Functions/Date_and_time_functions.md#YEARMONTH%28%29)
* [YEARMONTHDAY()](../Functions/Date_and_time_functions.md#YEARMONTHDAY%28%29)
* [WEIGHT()](../Functions/Searching_and_ranking_functions.md#WEIGHT%28%29)

## HTTP JSON中的表达式

在 HTTP JSON 接口中，表达式通过 `script_fields` 和 `expressions` 支持。

### script_fields

<!-- example script_fields -->
```json
{
	"index": "test",
	"query": {
		"match_all": {}
	}, "script_fields": {
		"add_all": {
			"script": {
				"inline": "( gid * 10 ) | crc32(title)"
			}
		},
		"title_len": {
			"script": {
	   			"inline": "crc32(title)"
			}
		}
	}
}
```

在这个例子中，创建了两个表达式：`add_all` 和 `title_len`。第一个表达式计算 `( gid * 10 ) | crc32(title)` 并将结果存储在 `add_all` 属性中。第二个表达式计算 `crc32(title)` 并将结果存储在 `title_len` 属性中。

目前，仅支持 `inline` 表达式。`inline` 属性的值（要计算的表达式）语法与 SQL 表达式相同。

表达式名称可以用于过滤或排序。


<!-- intro -->
##### script_fields:

<!-- request script_fields -->
```json
{
	"index":"movies_rt",
	"script_fields":{
		"cond1":{
			"script":{
				"inline":"actor_2_facebook_likes =296 OR movie_facebook_likes =37000"
			}
		},
		"cond2":{
			"script":{
				"inline":"IF (IN (content_rating,'TV-PG','PG'),2, IF(IN(content_rating,'TV-14','PG-13'),1,0))"
			}
		}
	},
	"limit":10,
	"sort":[
		{
			"cond2":"desc"
		},
		{
			"actor_1_name":"asc"
		},
		{
			"actor_2_name":"desc"
		}
	],
	"profile":true,
	"query":{
		"bool":{
			"must":[
				{
					"match":{
						"*":"star"
					}
				},
				{
					"equals":{
						"cond1":1
					}
				}
			],
			"must_not":[
				{
					"equals":{
						"content_rating":"R"
					}
				}
			]
		}
	}
}
```

<!-- end -->

默认情况下，表达式值会包含在结果集的 `_source` 数组中。如果源选择是选择性的（参见 [源选择](../Searching/Search_results.md#源选择)），表达式名称可以添加到请求中的 `_source` 参数中。请注意，表达式的名称必须为小写。

### expressions

<!-- example expressions -->

`expressions` 是 `script_fields` 的替代方法，语法更为简单。示例请求添加了两个表达式，并将结果存储在 `add_all` 和 `title_len` 属性中。请注意，表达式的名称必须为小写。

<!-- request expressions -->
```json
{
  "index": "test",
  "query": { "match_all": {} },
  "expressions":
  {
	  "add_all": "( gid * 10 ) | crc32(title)",
      "title_len": "crc32(title)"
  }
}
```

<!-- end -->

<!-- proofread -->
