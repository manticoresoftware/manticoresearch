# 搜索中的表达式

Manticore 通过 SQL 和 HTTP 支持使用任意算术表达式，包含属性值、内部属性（文档 ID 和相关性权重）、算术运算、多种内置函数和用户定义函数。以下是完整的参考列表，方便快速查阅。

## 算术运算符
```sql
+, -, *, /, %, DIV, MOD
```

支持标准算术运算符。涉及这些运算符的算术计算可以通过三种不同模式执行：

1. 使用单精度 32 位 IEEE 754 浮点数（默认），
2. 使用有符号 32 位整数，
3. 使用有符号 64 位整数。

表达式解析器会自动切换到整数模式，如果没有运算结果为浮点数。否则，使用默认的浮点模式。例如，a+b 如果两个参数都是 32 位整数，则使用 32 位整数计算；如果两个参数都是整数但其中一个是 64 位，则使用 64 位整数计算；否则使用浮点数计算。但是，`a/b` 或 `sqrt(a)` 总是以浮点数计算，因为这些运算返回非整数结果。为避免这种情况，可以使用 `IDIV(a,b)` 或 `DIV b` 形式。此外，`a*b` 在参数为 32 位时不会自动提升为 64 位。要强制使用 64 位结果，请使用 [BIGINT()](../Functions/Type_casting_functions.md#BIGINT%28%29)，但请注意，如果存在非整数运算，BIGINT() 会被忽略。

## 比较运算符
```sql
<, > <=, >=, =, <>
```

比较运算符在条件为真时返回 1.0，否则返回 0.0。例如，当属性 `a` 等于属性 `b` 时，`(a=b)+3` 计算结果为 4；当 `a` 不等于 `b` 时，结果为 3。与 MySQL 不同，等值比较（即 `=` 和 `<>` 运算符）包含一个小的相等阈值（默认 1e-6）。如果比较值之间的差异在阈值内，则视为相等。

对于多值属性，`BETWEEN` 和 `IN` 运算符如果至少有一个值满足条件，则返回真（类似于 [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)）。`IN` 运算符不支持 JSON 属性。`IS (NOT) NULL` 运算符仅支持 JSON 属性。

## 布尔运算符
```sql
AND, OR, NOT
```

布尔运算符（AND、OR、NOT）行为符合预期。它们是左结合的，并且优先级最低。NOT 的优先级高于 AND 和 OR，但仍低于其他运算符。AND 和 OR 具有相同优先级，建议在复杂表达式中使用括号以避免混淆。

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

## HTTP JSON 中的表达式

在 HTTP JSON 接口中，通过 `script_fields` 和 `expressions` 支持表达式。

### script_fields

<!-- example script_fields -->
```json
{
	"table": "test",
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

在此示例中，创建了两个表达式：`add_all` 和 `title_len`。第一个表达式计算 `( gid * 10 ) | crc32(title)` 并将结果存储在 `add_all` 属性中。第二个表达式计算 `crc32(title)` 并将结果存储在 `title_len` 属性中。

目前仅支持 `inline` 表达式。`inline` 属性的值（要计算的表达式）具有与 SQL 表达式相同的语法。

表达式名称可以用于过滤或排序。


<!-- intro -->
##### script_fields:

<!-- request script_fields -->
```json
{
	"table":"movies_rt",
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

默认情况下，表达式值包含在结果集的 `_source` 数组中。如果源是选择性的（参见[源选择](../Searching/Search_results.md#Source-selection)），可以将表达式名称添加到请求中的 `_source` 参数中。注意，表达式名称必须是小写。

### expressions

<!-- example expressions -->

`expressions` 是 `script_fields` 的一种替代方案，语法更简单。示例请求添加了两个表达式，并将结果存储到 `add_all` 和 `title_len` 属性中。注意，表达式名称必须是小写。

<!-- request expressions -->
```json
{
  "table": "test",
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

